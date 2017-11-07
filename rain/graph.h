#pragma once
#pragma once
#ifndef graph_h
#define graph_h
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>
#include <chrono>
#include <utility>
#include <dirent.h>


using namespace std;

extern unordered_map <string, double> voltages;
extern double vcrit;
int length_2(string node1, string node2);



struct segment {
	string cathode;
	string anode;
	double resistor;
	double sigma;

	double delta_v;
};

class Graph
{
public:
	unordered_map <string, vector<pair<string, double>>> matrix; //matrix[node_name]={(adj_node_name, resistor),(adj_node_name, resistor),...}
	unordered_map <string, bool> visit;
	int nCC;
	vector<double> vsigma;
	unordered_map <string, int> CC; //CC[node_name]=CC_number

									///
	vector<segment> segment_to_be_fixed;

	Graph();
	void DFSUtil2(string node, unordered_map <string, bool> &visit, double &sigma, double &volume);
	void addEdge2(string vs, string ws, double r);
	void connectedComponents();
};

#endif
