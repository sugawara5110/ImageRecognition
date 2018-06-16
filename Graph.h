//*****************************************************************************************//
//**                                                                                     **//
//**                                    Graph                                            **//
//**                                                                                     **//
//*****************************************************************************************//
#ifndef Class_Graph_Header
#define Class_Graph_Header

#include "../../../Common/Direct3DWrapper/Dx_NN.h"
#include "../../../Common/Direct3DWrapper/DxText.h"

class Graph {

private:
	UINT **point;
	PolygonData2D *graph;
	int piw, pih;

public:
	Graph();
	~Graph();
	void CreateGraph(float x, float y, float w, float h, int pw, int ph);
	void Clear();
	void SetData(int cnt, int data, UINT col);
	void Draw();
};

#endif

