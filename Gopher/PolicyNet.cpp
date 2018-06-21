#include "PolicyNet.h"
#include "defines.h"

#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define COMPILER_MSVC
#define NOMINMAX
#include <tensorflow\core\public\session.h>
#include <tensorflow\core\protobuf\meta_graph.pb.h>
#include <tensorflow\core\framework\tensor.h>
#include <tensorflow\cc\ops\standard_ops.h>
//#include <tensorflow\cc\client\client_session.h>

namespace tf = tensorflow;

// Move these to defines once relevent for storage
static constexpr int LayersPerState = 2;
static constexpr int BoardHistory = 3;
static constexpr int BoardDepth = LayersPerState * BoardHistory + 1;
static constexpr int InputSize = BoardDepth * BoardSize2;

static const std::string pathToCheckpoint = "Models/PolicyModel/latestModel";
static const std::string pathToGraph = pathToCheckpoint + ".meta";

//static const std::string pathToGraph = "Models/PolicyModel/graph.pb";

#include <iomanip>
#include <tensorflow\cc\saved_model\loader.h>

namespace PolicyNet
{
	tf::Status status;
	tf::Session* session;
	tf::SessionOptions options;
	tf::MetaGraphDef graphDef;

// TODO: Make the net optional
void init()
{
	// Set memory growth manually. Resolves and issue with tf 1.5 for windows
	double memFraction = 1.0;
	bool allowMemGrowth = true;
	options.config.mutable_gpu_options()->set_allow_growth(allowMemGrowth);
	options.config.mutable_gpu_options()->set_per_process_gpu_memory_fraction(memFraction);

	session = tf::NewSession(options);
	if (!session)
		throw std::runtime_error("Could no create Tensorflow Session!");

	// Read in the protobuf
	status = tf::ReadBinaryProto(tf::Env::Default(), pathToGraph, &graphDef);
	if (!status.ok())
		throw std::runtime_error("Error reading graph: " + status.ToString());

	status = session->Create(graphDef.graph_def());
	if (!status.ok())
		throw std::runtime_error("Error creating graph: " + status.ToString());

	// Read the weights
	tf::Tensor checkpointPathTensor(tf::DT_STRING, tf::TensorShape());
	checkpointPathTensor.scalar<std::string>()() = pathToCheckpoint;
	
	status = session->Run(
		{ { graphDef.saver_def().filename_tensor_name(), checkpointPathTensor }, },
		{},
		{ graphDef.saver_def().restore_op_name() },
		nullptr
	);
	
	if (!status.ok())
		throw std::runtime_error("Error loading checkpoint from " + pathToCheckpoint + ": " + status.ToString());

	run();
}

void run()
{
	
	static const std::string inputName = "Input_input"; 
	static const std::string outputName = "Output/Softmax:0";

	static const auto shape = tf::TensorShape({ 1, BoardDepth, BoardSize, BoardSize });

	// Placeholder until we're using actual input data
	float inData[InputSize] = { 0.f };

	tf::Tensor input(tf::DT_FLOAT, shape);
	std::copy_n(inData, InputSize, input.flat<float>().data());

	std::vector<tf::Tensor> outputs;
	status = session->Run({ { inputName, input } }, { outputName }, {}, &outputs);

	tf::TTypes<float>::Flat flatOut = outputs[0].flat<float>();

	for (int i = 0; i < BoardSize2; ++i)
	{
		if (i % BoardSize == 0)
			std::cout << '\n';
		std::cout << std::setw(8) << std::fixed << std::setprecision(5) << flatOut(i) * 100.0 << ", ";
	}

	int a = 5;
	/*
	// and run the inference to your liking
	auto feedDict = ...
	auto outputOps = ...
	status = session->Run(feedDict, outputOps, {}, &outputTensors);

	*/
}









}
