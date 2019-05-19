//*****************************************************************************************//
//**                                                                                     **//
//**                                    Graph                                            **//
//**                                                                                     **//
//*****************************************************************************************//

#include "Graph.h"

Graph::Graph() {
	graph = new PolygonData2D();
	graph->GetVBarray2D(1);
	graph->SetCommandList(0);
}

Graph::~Graph() {
	S_DELETE(graph);
	for (int i = 0; i < pih; i++)ARR_DELETE(point[i]);
	ARR_DELETE(point);
}

void Graph::CreateGraph(float x, float y, float w, float h, int pw, int ph) {
	piw = pw;
	pih = ph;
	graph->TexOn();
	graph->TextureInit(pw, ph);
	graph->CreateBox(x, y, 0.1f, w, h, 1.0f, 1.0f, 1.0f, 1.0f, TRUE, TRUE);
	point = new UINT*[ph];
	for (int i = 0; i < ph; i++)point[i] = new UINT[pw];
	Clear();
}

void Graph::Clear() {
	for (int j = 0; j < pih; j++) {
		for (int i = 0; i < piw; i++) {
			point[j][i] = 0xffff0000;
		}
	}
}

void Graph::SetData(int cnt, int data, UINT col) {
	point[data][cnt] = col;
}

void Graph::Draw() {
	graph->Update(0, 0, 0, 0, 0, 0, 0, 1.0f, 1.0f);
	graph->SetTextureMPixel(point, 0xff, 0xff, 0xff, 0xff, 0);
	graph->Draw();
}