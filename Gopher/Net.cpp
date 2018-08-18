#include "Net.h"
#include "defines.h"
#include "GameState.h"
#include <cntk\Include\CNTKLibrary.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "RedirectStreams.h"
#include "Options.h"

namespace Net
{
	CNTK::FunctionPtr	model;
	CNTK::Variable		inputVar;
	CNTK::Variable		policyOut;
	CNTK::Variable		valueOut;

	//const auto& device = CNTK::DeviceDescriptor::UseDefaultDevice();
	const auto& device = CNTK::DeviceDescriptor::GPUDevice(0);
	//const auto& device = CNTK::DeviceDescriptor::CPUDevice(); 

	// Allow the default device to be decided at runtime through
	// cmdLine options or cfg file
	//auto* device = &CNTK::DeviceDescriptor::UseDefaultDevice();

void init()
{
	// TODO: Check load status here too? Add more descrptive failures?
	std::ifstream ifs(options.path, std::ifstream::in | std::ifstream::binary);
	
	try
	{
		model = CNTK::Function::Load(ifs, device);
	}
	catch (const std::runtime_error& error)
	{
		std::cerr << "Failed to load CNTK model! " << error.what() << '\n';
	}
	inputVar	= model->Arguments()[0];
	policyOut	= model->Outputs()[NetResult::MOVES];
	valueOut	= model->Outputs()[NetResult::WIN_CHANCE];

	// TODO: Now we need to 
	// 1: Figure out how we want to store the binary board states
	// 2: Implement the storing or creation of the needed inputs
	// 3: Figure out what orientation the flat inputVar takes (compared to the models 3D input)
}

template<class T>
void printNetOut(size_t size, const std::vector<std::vector<T>>& outputBuffer)
{
	std::cerr << '\n';
	const auto& out = outputBuffer[NetResult::MOVES];
	for (auto i = 0; i < out.size(); ++i)
	{
		if (i % 19 == 0 && i != 0)
			std::cerr << '\n';

		std::cerr << std::fixed << std::setprecision(2)
			<< std::setw(6) << out[i] << ", ";
	}
	std::cerr << '\n';
	std::cerr << outputBuffer[1][0] << ", " << outputBuffer[1][1];
	std::cerr << '\n';
}

NetResult inference(const GameState& state, int color)
{
	// TODO: Look into caching these inputs
	NetInput input(state, color);

	// TODO: CNTK seems to crash an inordinate amount here,
	// seems to happen more if device gets init and pauses before inputVal
	// figure out why and if we can prevent it!
	// Crash seems to happen or be related to cpp std:: memory file
	auto inputVal = CNTK::Value::CreateBatch(inputVar.Shape(), input.slices, device);
	std::unordered_map<CNTK::Variable, CNTK::ValuePtr> inputDataMap = { { inputVar, inputVal } };

	// Create output data map. Using null as Value to indicate using system allocated memory.
	// Alternatively, create a Value object and add it to the data map.
	std::unordered_map<CNTK::Variable, CNTK::ValuePtr> outputDataMap = { { policyOut, nullptr }, { valueOut, nullptr } };

	// Evaluate!
	model->Evaluate(inputDataMap, outputDataMap, device);

	// TODO: Efficiency here can definitely be improved.
	// Allocating memory twice seems stupid
	int i = 0;
	NetResult result;
	for (const auto& outputPair : outputDataMap)
	{
		const auto& var = outputPair.first;
		const auto& val = outputPair.second;
		std::vector<std::vector<float>> outputData;
		val->CopyVariableValueTo(var, outputData);
		std::move(std::begin(outputData[0]), std::end(outputData[0]), std::begin(result.output[i++]));
	}
	result.process();

	if (result.output.size() == 0)
		int a = 5;

	// Debugging only!
	//printNetOut(361, result.output);

	return result;
}

}
