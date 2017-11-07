#include "graph.h"


//ofstream cfile("spice/ibmpg2/calcs.txt");


/*template<typename Out>
void split(const std::string &s, char delim, Out result) {
std::stringstream ss;
ss.str(s);
std::string item;
while (std::getline(ss, item, delim)) {
*(result++) = item;
}
}

std::vector<std::string> split(const std::string &s, char delim) {
std::vector<std::string> elems;
split(s, delim, std::back_inserter(elems));
return elems;
}

int fast_atoi(const char * str)
{
int val = 0;
while (*str) {
val = val * 10 + (*str++ - '0');
}
return val;
}*/



int length_2(string node1, string node2)
{
	stringstream ss1, ss2;
	ss1.str(node1);
	ss2.str(node2);
	string x1, y1, x2, y2;
	getline(ss1, x1, '_');
	getline(ss1, x1, '_');
	getline(ss1, y1, '_');
	getline(ss2, x2, '_');
	getline(ss2, x2, '_');
	getline(ss2, y2, '_');
	int dx1, dy1, dx2, dy2;
	istringstream osx1(x1), osy1(y1), osx2(x2), osy2(y2);
	osx1 >> dx1;osy1 >> dy1;osx2 >> dx2;osy2 >> dy2;
	int l = dx1 - dx2 + dy1 - dy2;
	return l*l;

	/*vector<string> vnode1 = split(node1, '_'); vector<string> vnode2 = split(node2, '_');
	int xx1 = fast_atoi(vnode1[1].c_str());	int yy1 = fast_atoi(vnode1[2].c_str());	int xx2 = fast_atoi(vnode2[1].c_str());	int yy2 = fast_atoi(vnode2[2].c_str());
	int l = xx1 - xx2 + yy1 - yy2;	return l*l;*/
}

Graph::Graph()
{
	nCC = 0;
}

void Graph::addEdge2(string vs, string ws, double r)
{
	pair<string, double> wr(ws, r);
	pair<string, double> vr(vs, r);
	matrix[vs].push_back(wr);
	matrix[ws].push_back(vr);
}

void Graph::DFSUtil2(string node, unordered_map <string, bool> &visit, double &sigma, double &volume)
{
	CC[node] = nCC;
	visit[node] = true;
	double branch_volume;double term;
	for (auto& i : matrix[node]) {
		if (!visit[i.first])
			DFSUtil2(i.first, visit, sigma, volume);
		branch_volume = length_2(node, i.first) / i.second; // V=L*A=L*(rho*L/R) since rho will be canceld by the rho in denominator (sum of volumes) so V_simplified = L^2/R
		term = (voltages[node] + voltages[i.first]) * branch_volume;
		volume += branch_volume;
		sigma += term;
		/*if (nCC == 0) {//0 for ibmpg0
		cfile << node << " - " << i.first << ": "
		<< " branch_volume=L^2/R=" << length_2(node, i.first) << "/" << i.second << "=" << branch_volume
		<< " | term=" << "v(" << node << ")+(" << i.first << ")*branch_volume=(" << voltages[node] <<"+"<< voltages[i.first] <<")*" << branch_volume<< "=" << term
		<< " | volume=" << volume
		<< " |sigma=" << sigma << endl;
		}*/
	}
}

void Graph::connectedComponents()
{
	double sigma;
	double volume;
	for (auto& node : matrix)
		visit[node.first] = false;
	for (auto& node : matrix) {
		if (visit[node.first] == false) {
			sigma = 0;volume = 0;
			DFSUtil2(node.first, visit, sigma, volume);
			vsigma.push_back(sigma / volume); //there is no /2 at all, we double count in sigma and volume each edge, they cancel each other at the end
			nCC++;
		}
	}
}
