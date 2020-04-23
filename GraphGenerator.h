/*
 * GraphGenerator.h
 *
 *  Created on: 2014/08/06
 *      Author: sasaki
 */

#ifndef GRAPHGENERATOR_H_
#define GRAPHGENERATOR_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Parameters.h"
#include "generalTools.h"

using namespace std;



const int YET=0;
const int DONE=1;
const int LABEL=2;
const int CONNECT=2;

class Node
{
private:
	int id;
public:
	Node();
	Node(int);
	~Node();
};

class DijkstraNode: public Node
{
public:
	bool done;
};

class Edge
{
private:
	int id;
	int nodeStart;
	int nodeEnd;
public:
	Edge();
	~Edge();
};


void graphGenerate(int edge[NODE_NUM][NODE_NUM], int label[NODE_NUM]);
void graphFileGenerate(FILE *, FILE*);

#endif /* GRAPHGENERATOR_H_ */
