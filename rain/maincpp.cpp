#include "graph.h"

unordered_map <string, double> voltages;
double vcrit = 0.014513;//0.013477;//0.01244;//0.0114;

pair<string, string> xy(string node)
{
	stringstream ss1, ss2;
	ss1.str(node);
	string x, y;
	getline(ss1, x, '_');
	getline(ss1, x, '_');
	getline(ss1, y, '_');
	return std::make_pair(x, y);

	/*vector<string> vnode1 = split(node1, '_'); vector<string> vnode2 = split(node2, '_');
	int xx1 = fast_atoi(vnode1[1].c_str());	int yy1 = fast_atoi(vnode1[2].c_str());	int xx2 = fast_atoi(vnode2[1].c_str());	int yy2 = fast_atoi(vnode2[2].c_str());
	int l = xx1 - xx2 + yy1 - yy2;	return l*l;*/
}

bool is_cathode_only(string via, Graph& g) {
	for (auto& adj : g.matrix[via]) {
		if (voltages[adj.first] < voltages[via])
			return false;
	}
	return true;
}
void read_voltages(int testcase) {
	ifstream vfile("spice/ibmpg" + to_string(testcase) + "/ibmpg" + to_string(testcase) + ".solution");
	string node_name;
	string node_svoltage;
	double node_voltage;
	istringstream os;
	while (vfile >> node_name >> node_svoltage) {
		os.clear();os.str(node_svoltage);
		os >> node_voltage;
		voltages[node_name] = node_voltage;
	}
}

void breakdown_spice_file(int testcase) {
	ifstream lfile("spice/ibmpg" + to_string(testcase) + "/ibmpg" + to_string(testcase) + ".spice");
	ofstream outv0("spice/ibmpg" + to_string(testcase) + "/breakdown/gndsources.txt");
	ofstream outv1("spice/ibmpg" + to_string(testcase) + "/breakdown/vddsources.txt");
	ofstream out("garbage.txt");
	string str;
	int k = 0;
	while (getline(lfile, str)) {
		if (str[0] == 'R' || str[0] == 'V')
			out << str << endl;
		else if (str[0] == 'v') {
			if (str.back() == '0')
				outv0 << str << endl;
			else
				outv1 << str << endl;
		}
		else if (str[2] == 'l' || str[2] == 'v') {
			str.erase(0, 1);
			str.erase(std::remove(str.begin(), str.end(), ':'), str.end());
			std::replace(str.begin(), str.end(), ' ', '_');
			std::replace(str.begin(), str.end(), ',', '_');
			++k;
			out.close();
			out.open("spice/ibmpg" + to_string(testcase) + "/breakdown/" + str + ".txt");
		}
	}
}


int main() {
	//!!!!! BREAKDOWN IS DONE USING A SEPARATE PROGRAM

	/*testcase = 9; net =2*/
	//vcrit = 0.014513;//0.013477;//0.01244;//0.0114;
	int testcase, net_number;
	//cout << "Enter IBMPG testcase number: ";cin >> testcase;
	//cout << "Enter net number: ";cin >> net_number;
	for (testcase = 0; testcase < 1; testcase++) { //0 to 7 //for detection testing 2 < 3 for repair testing 0 < 1
		cout << "TESTCASE: " << testcase << endl;

		DIR *dir;
		struct dirent *ent;
		vector<string> filenames;
		string directory = "spice\\ibmpg" + to_string(testcase) + "\\breakdown\\";
		if ((dir = opendir(directory.c_str())) != NULL) {/* print all the files and directories within directory */
			while ((ent = readdir(dir)) != NULL) {
				string strfile(ent->d_name);
				filenames.push_back(strfile);
			}
			closedir(dir);
		}
		else {/* could not open directory */
			perror("");
			return EXIT_FAILURE;
		}
		cout << "READ VOLTAGES - START" << endl;
		read_voltages(testcase);
		cout << "READ VOLTAGES - END\n" << endl;

		int net_max = -1;
		for (auto& x : filenames)
			if (x.find("layer") != string::npos)
				net_max++;

		for (net_number = 0; net_number <= 0; net_number++) { // 0 to net_max
			cout << "NET: " << net_number << endl;

			vector<string> netfiles;
			std::copy_if(filenames.begin(), filenames.end(), std::back_inserter(netfiles),
				[net_number](std::string const& str) { return str.find("_" + to_string(net_number)) != string::npos; });
			string netfile;
			string viafile1;
			string viafile2;
			for (auto const& value : netfiles) {
				if (value.find("layer") != string::npos)
					netfile = value;
				else if (value.find(to_string(net_number) + "_to_") != string::npos)
					viafile1 = value;
				else if (value.find("_to_" + to_string(net_number)) != string::npos)
					viafile2 = value;
			}


			cout << "Net and via files: " << netfile << ", " << viafile1 << ", " << viafile2 << endl << endl;



			cout << "BUILDING GRAPH - START" << endl;
			ifstream nfile("spice/ibmpg" + to_string(testcase) + "/breakdown/" + netfile);
			Graph g;
			string rns, fs, ts, rs;double r;
			istringstream oss;
			while (nfile >> rns >> fs >> ts >> rs) {
				oss.clear();oss.str(rs);oss >> r;
				g.addEdge2(fs, ts, r);
			}
			cout << "BUILDING GRAPH - END\n" << endl;

			//testing graph
			/*for (auto& node : g.matrix) {
			tfile << node.first << ": ";
			for(auto& neighbour : g.matrix[node.first])
			tfile << neighbour.first << " " ;
			tfile << endl;
			}*/

			cout << "CC - START \n";
			std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
			g.connectedComponents();
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;
			cout << "CC - END \n\n";

			//via reading
			ifstream vfile1("spice/ibmpg" + to_string(testcase) + "/breakdown/" + viafile1);
			ifstream vfile2("spice/ibmpg" + to_string(testcase) + "/breakdown/" + viafile2);
			ifstream vsrcfile("garbage.txt");
			vsrcfile.close();
			if (netfile.find("GND") != std::string::npos) {
				vsrcfile.open("spice/ibmpg" + to_string(testcase) + "/breakdown/" + "gnd_v_sources.txt");
			}
			else {
				vsrcfile.open("spice/ibmpg" + to_string(testcase) + "/breakdown/" + "vdd_v_sources.txt");
			}
			ifstream isrcfile("garbage.txt");
			isrcfile.close();
			if (netfile.find("GND") != std::string::npos) {
				isrcfile.open("spice/ibmpg" + to_string(testcase) + "/breakdown/" + "gnd_i_sources.txt");
			}
			else {
				isrcfile.open("spice/ibmpg" + to_string(testcase) + "/breakdown/" + "vdd_i_sources.txt");
			}
			string vrns, vfs, vts, vrs;
			vector<string> vias;
			unordered_map<string, bool> viaz;
			while (vfile1 >> vrns >> vfs >> vts >> vrs) {
				if (g.matrix.find(vfs) != g.matrix.end()) // might be dummy via no connection to net, so no current, so that node cannot be cathode
														  //vias.push_back(vfs);
					viaz[vfs];
			}
			while (vfile2 >> vrns >> vfs >> vts >> vrs) {
				if ((g.matrix.find(vts) != g.matrix.end()) /* && (find(vias.begin(), vias.end(), vts) == vias.end())*/) // might be dummy via no connection to net, so no current, so that node cannot be cathode
																														//vias.push_back(vts);
					viaz[vts];
			}
			while (vsrcfile >> vrns >> vfs >> vts >> vrs) {
				vfs.erase(0, 3);
				if ((g.matrix.find(vfs) != g.matrix.end()) /*&& (find(vias.begin(), vias.end(), vfs) == vias.end())*/)
					//vias.push_back(vfs);
					viaz[vfs];
			}
			if (net_number == 0) { //net0 -- i sources are considered only for net 0 and 1
				while (isrcfile >> vrns >> vfs >> vts >> vrs) {
					if ((g.matrix.find(vts) != g.matrix.end()) /*&& (find(vias.begin(), vias.end(), vfs) == vias.end())*/)
						//vias.push_back(vfs);
						viaz[vts];
				}
			}
			else if (net_number == 1) { //net1 -- i sources are considered only for net 0 and 1
				while (isrcfile >> vrns >> vfs >> vts >> vrs) {
					if ((g.matrix.find(vfs) != g.matrix.end()) /*&& (find(vias.begin(), vias.end(), vfs) == vias.end())*/)
						//vias.push_back(vfs);
						viaz[vfs];
				}
			}
			/*while (isrcfile >> vrns >> vfs >> vts >> vrs) {
			if (vfs == "0") {
			if ((g.matrix.find(vts) != g.matrix.end())); //&& (find(vias.begin(), vias.end(), vfs) == vias.end())
			//vias.push_back(vfs);
			viaz[vts];
			}
			else {
			if ((g.matrix.find(vfs) != g.matrix.end())); //&& (find(vias.begin(), vias.end(), vfs) == vias.end())
			//vias.push_back(vfs);
			viaz[vfs];
			}

			}*/

			ofstream rfile("spice/ibmpg" + to_string(testcase) + "/report_net_" + to_string(net_number) + ".txt");
			cout << "NODE VIOLATIONS - START \n";
			ofstream tfile("spice/ibmpg" + to_string(testcase) + "/violations_net_" + to_string(net_number) + ".txt");
			ofstream mmmfile("spice/ibmpg" + to_string(testcase) + "/node_violations_xyv_net_" + to_string(net_number) + ".txt");
			ofstream mmmmfile("spice/ibmpg" + to_string(testcase) + "/node_ratio_xyv_net_" + to_string(net_number) + ".txt");

			vector<string> node_violated;
			int num_node_violations = 0;
			int nodd = 0;
			pair<string, string> vn;
			segment segg;
			segg.sigma = 0.0;
			segg.delta_v = 0.0;
			for (int j = 0; j < g.nCC; j++) //making all sigma for segment to fixed for all cc zero 
				g.segment_to_be_fixed.push_back(segg);
			ofstream dfile("spice/ibmpg" + to_string(testcase) + "/debug_" + to_string(net_number) + ".txt");

			for (auto& nodee : g.matrix) {
				++nodd;
				vn = xy(nodee.first);
				if (g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first] > vcrit) {
					++num_node_violations;
					tfile << "node violation @ " << nodee.first << " (cc = " << g.CC[nodee.first] << ") " << g.vsigma[g.CC[nodee.first]] << " - 2*" << voltages[nodee.first] << " = " << g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first] << " > " << vcrit << " (" << (g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first]) / vcrit << ")" << endl;
					node_violated.push_back(nodee.first);
					mmmfile << vn.first << " " << vn.second << " " << (g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first]) / vcrit << endl;
					//DEBUG		if (g.CC[nodee.first]==0) {	dfile << "node violation @ " << nodee.first << " (cc = " << g.CC[nodee.first] << ") " << g.vsigma[g.CC[nodee.first]] << " - 2*" << voltages[nodee.first] << " = " << g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first] << " > " << vcrit << " (" << (g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first]) / vcrit << ")" << endl;	}

					//to fin max (v_a - v_c) NEEDED FOR SOME HEURISTICS
					/*
					double branch_volume; double term;
					for (auto& nodee_neighbour : nodee.second) {
					//nodee.first; //vilated node name
					//nodee_neighbour.first; //neighbour node name
					//nodee_neighbour.second;// seg resistor
					if (voltages[nodee.first] < voltages[nodee_neighbour.first]) { // if it's the cathode to the neighbour (i.e. outgoing flow)
					term = (voltages[nodee_neighbour.first]- voltages[nodee.first]);
					if (term > g.segment_to_be_fixed[g.CC[nodee.first]].delta_v) {
					if (g.CC[nodee.first] == 0) {
					cout << term << " - " << g.segment_to_be_fixed[g.CC[nodee.first]].delta_v << " - " << nodee.first << " - " << nodee_neighbour.first << endl;
					}
					g.segment_to_be_fixed[g.CC[nodee.first]].delta_v = term;
					g.segment_to_be_fixed[g.CC[nodee.first]].cathode = nodee.first;
					g.segment_to_be_fixed[g.CC[nodee.first]].anode = nodee_neighbour.first;
					g.segment_to_be_fixed[g.CC[nodee.first]].resistor = nodee_neighbour.second;
					}
					}
					}
					*/

					//to find max (v_c + v_a)*V NEEDED FOR SOME HEURISTICS
					/*
					double branch_volume; double term;
					for (auto& nodee_neighbour : nodee.second) {
					//nodee.first; //vilated node name
					//nodee_neighbour.first; //neighbour node name
					//nodee_neighbour.second;// seg resistor
					if (voltages[nodee.first] < voltages[nodee_neighbour.first]) { // if it's the cathode to the neighbour (i.e. outgoing flow)
					branch_volume = length_2(nodee.first, nodee_neighbour.first) / nodee_neighbour.second;
					term = (voltages[nodee.first] + voltages[nodee_neighbour.first]) * branch_volume;
					if (term > g.segment_to_be_fixed[g.CC[nodee.first]].sigma) {
					if (g.CC[nodee.first] == 0) {
					cout << term << " - " << g.segment_to_be_fixed[g.CC[nodee.first]].sigma << " - "  << nodee.first << " - " << nodee_neighbour.first << endl;
					}
					g.segment_to_be_fixed[g.CC[nodee.first]].sigma = term;
					g.segment_to_be_fixed[g.CC[nodee.first]].cathode = nodee.first;
					g.segment_to_be_fixed[g.CC[nodee.first]].anode = nodee_neighbour.first;
					g.segment_to_be_fixed[g.CC[nodee.first]].resistor = nodee_neighbour.second;
					}
					}
					}
					*/
				}
				mmmmfile << vn.first << " " << vn.second << " " << (g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first]) / vcrit << endl;
				///DEBUG
				dfile << "node violation @ " << nodee.first << " (cc = " << g.CC[nodee.first] << ") " << g.vsigma[g.CC[nodee.first]] << " - 2*" << voltages[nodee.first] << " = " << g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first] << " > " << vcrit << " (" << (g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first]) / vcrit << ")" << endl;
				///DEBUG
			}
			cout << num_node_violations << " nodes are EM prone, out of " << nodd << " nodes" << endl;
			rfile << num_node_violations << " nodes are EM prone, out of " << nodd << " nodes" << endl;
			cout << "NODE VIOLATIONS - END \n\n";

			cout << "VIA VIOLATIONS - START \n";
			ofstream mfile("spice/ibmpg" + to_string(testcase) + "/via_violations_xyv_net_" + to_string(net_number) + ".txt");
			ofstream mmfile("spice/ibmpg" + to_string(testcase) + "/via_ratio_xyv_net_" + to_string(net_number) + ".txt");
			vector<string> via_violated;
			int num_via_violations = 0;
			int nvias = 0;
			pair<string, string> vv;
			for (auto& nodee : viaz/*vias*/) {
				++nvias;
				vv = xy(nodee.first);
				if (g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first] > vcrit) {
					++num_via_violations;
					tfile << "via violation @ " << nodee.first << " (cc = " << g.CC[nodee.first] << ") " << g.vsigma[g.CC[nodee.first]] << " - 2*" << voltages[nodee.first] << " = " << g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first] << " > " << vcrit << " (" << (g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first]) / vcrit << ")" << endl;
					via_violated.push_back(nodee.first);
					mfile << vv.first << " " << vv.second << " " << (g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first]) / vcrit << endl;
				}
				mmfile << vv.first << " " << vv.second << " " << (g.vsigma[g.CC[nodee.first]] - 2 * voltages[nodee.first]) / vcrit << endl;
				//else tfile << "no violation @ " << nodee << " (cc = " << g.CC[nodee] << ") " << g.vsigma[g.CC[nodee]] << " - 2*" << voltages[nodee] << " = " << g.vsigma[g.CC[nodee]] - 2 * voltages[nodee] << " < " << vcrit << endl;
			}
			cout << num_via_violations << " vias are EM prone, out of " << nvias << " vias" << endl;
			rfile << num_via_violations << " vias are EM prone, out of " << nvias << " vias" << endl;
			cout << "VIA VIOLATIONS - END \n\n";

			cout << "FIXING - START \n";
			vector<string> to_be_fixed;
			int nfix = 0;
			for (auto& nodee : via_violated) {
				if (is_cathode_only(nodee, g)) {
					to_be_fixed.push_back(nodee);
					tfile << "repair @ " << nodee << " (cc = " << g.CC[nodee] << ") " << g.vsigma[g.CC[nodee]] << " - 2*" << voltages[nodee] << " = " << g.vsigma[g.CC[nodee]] - 2 * voltages[nodee] << " > " << vcrit << " (" << (g.vsigma[g.CC[nodee]] - 2 * voltages[nodee]) / vcrit << ")" << endl;
					nfix++;
				}
			}
			cout << nfix << " vias must be fixed, out of " << num_via_violations << " violated vias" << endl;
			rfile << nfix << " vias must be fixed, out of " << num_via_violations << " violated vias" << endl;
			cout << "FIXING - END \n";


			cout << "REPAIR RECOMMENDATION - START \n";
			segment seg;
			for (int i = 0; i < 1 /*g.nCC*/; i++) {
				seg = g.segment_to_be_fixed[i];
				//dfile << i << ": " << seg.cathode << " - " << seg.anode << " - " << seg.resistor << " (sigma = " << seg.sigma << " | ratio = " << (g.vsigma[g.CC[seg.cathode]] - 2 * voltages[seg.cathode]) / vcrit  << ")"<< endl;
				dfile << i << ": " << seg.cathode << " - " << seg.anode << " - " << seg.resistor << " (delta_v = " << seg.delta_v << " | ratio = " << (g.vsigma[g.CC[seg.cathode]] - 2 * voltages[seg.cathode]) / vcrit << ")" << endl;
			}
			cout << "REPAIR RECOMMENDATION - END \n";
			/*
			ofstream fff("xyz.txt");
			int min = 0;
			int max = 10000;
			int i = 0;
			double fmax = 1.0;
			while (i++ < 1000)
			fff << min + (rand() % (int)(max - min + 1)) << " " << min + (rand() % (int)(max - min + 1)) << " " << static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / fmax)) << endl;
			*/
		}
	}
	return 0;
}
