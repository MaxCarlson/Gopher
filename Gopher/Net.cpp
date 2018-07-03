#include "Net.h"
#include "defines.h"
#include "GameState.h"
#include <cntk\Include\CNTKLibrary.h>
#include <fstream>
#include <iostream>
#include <iomanip>


static const std::string modelPath = "models/PolicyModel/";
static const std::string fileName = "GoNet.dnn";

namespace Net
{
	CNTK::FunctionPtr		 model;
	CNTK::DeviceDescriptor*  device;
	CNTK::Variable		     inputVar;
	CNTK::Variable			 outputVar;

void init()
{
	std::ifstream ifs(modelPath + fileName, std::ifstream::in | std::ifstream::binary);

	model     = CNTK::Function::Load(ifs);
	device    = &CNTK::DeviceDescriptor::UseDefaultDevice();
	inputVar  = model->Arguments()[0];
	outputVar = model->Output();

	// TODO: Train two heads as in alpha go, one for value of board state one for policy to move

	// Just for testing
	//auto shp = inputVar.Shape();

	// TODO: Now we need to 
	// 1: Figure out how we want to store the binary board states
	// 2: Implement the storing or creation of the needed inputs
	// 3: Figure out what orientation the flat inputVar takes (compared to the models 3D input)

	run();
}

template<class T>
void printNetOut(size_t size, const std::vector<std::vector<T>>& outputBuffer)
{
	std::cout << '\n';
	auto out = outputBuffer[0];
	for (auto i = 0; i < out.size(); ++i)
	{
		if (i % 19 == 0 && i != 0)
			std::cout << '\n';

		std::cout << std::fixed << std::setprecision(2)
			<< std::setw(6) << out[i] << ", ";
	}
	std::cout << '\n';
}

void testRun()
{
	std::vector<float> inputData(inputVar.Shape().TotalSize());
	std::fill(inputData.begin(), inputData.end(), 0.f);

	// Create input value and input data map
	auto inputVal = CNTK::Value::CreateBatch(inputVar.Shape(), inputData, *device);
	std::unordered_map<CNTK::Variable, CNTK::ValuePtr> inputDataMap = { { inputVar, inputVal } };

	// Create output data map. Using null as Value to indicate using system allocated memory.
	// Alternatively, create a Value object and add it to the data map.
	std::unordered_map<CNTK::Variable, CNTK::ValuePtr> outputDataMap = { { outputVar, nullptr } };

	// Evaluate!
	model->Evaluate(inputDataMap, outputDataMap, *device);

	auto outputVal = outputDataMap[outputVar];
	std::vector<std::vector<float>> outputData;
	outputVal->CopyVariableValueTo(outputVar, outputData);

	printNetOut(outputVar.Shape().TotalSize(), outputData);
}

void run()
{
	testRun();
}

}
