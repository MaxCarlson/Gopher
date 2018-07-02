#include "PolicyNet.h"
#include "defines.h"
#include <cntk\Include\CNTKLibrary.h>
#include <fstream>
#include <iostream>
#include <iomanip>


// Move these to defines once relevent for storage
static constexpr int LayersPerState = 2;
static constexpr int BoardHistory = 3;
static constexpr int BoardDepth = LayersPerState * BoardHistory + 1;
static constexpr int InputSize = BoardDepth * BoardSize2;

static const std::string modelPath = "models/PolicyModel/";
static const std::string fileName = "GoNet.dnn";

namespace PolicyNet
{
	CNTK::FunctionPtr		model;
	CNTK::DeviceDescriptor*  device;
	CNTK::Variable		    inputVar;
	CNTK::Variable			outputVar;

void init()
{
	std::ifstream ifs(modelPath + fileName, std::ifstream::in | std::ifstream::binary);

	model     = CNTK::Function::Load(ifs);
	device    = &CNTK::DeviceDescriptor::UseDefaultDevice();
	inputVar  = model->Arguments()[0];
	outputVar = model->Output();

	// Just for testing
	auto shp = inputVar.Shape();

}

void run()
{

}

}
