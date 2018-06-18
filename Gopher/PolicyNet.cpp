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

static const std::string pathToCheckpoint = "Models/PolicyModel/latestModel";
static const std::string pathToGraph = pathToCheckpoint + ".meta";

#include <iomanip>


namespace PolicyNet
{
	tf::Status status;
	tf::Session* session;
	tf::SessionOptions options;
	tf::MetaGraphDef graphDef;

void init()
{
	session = tf::NewSession(options);
	//TF_CHECK_OK(tf::NewSession(options, &session));

	// TODO: Make the net optional
	if (!session)
		throw std::runtime_error("Could no create Tensorflow Session!");

	// Read in the protobuf
	status = tf::ReadBinaryProto(tf::Env::Default(), pathToGraph, &graphDef);
	if (!status.ok())
		throw std::runtime_error("Error creating graph: " + status.ToString());


	//status = session->Create(graphDef);
	status = session->Create(graphDef.graph_def());

	// Read the weights
	tf::Tensor checkpointPathTensor(tf::DT_STRING, tf::TensorShape());
	checkpointPathTensor.scalar<std::string>()() = pathToCheckpoint;

	
	
	//auto ss = graphDef.saver_def().filename_tensor_name();
	//auto sss = graphDef.saver_def().restore_op_name();
	//int sz = graphDef.graph_def().node_size();
	//std::vector<std::string> vNames;
	//for (int i = 0; i < sz; i++) {
	//	auto n = graphDef.graph_def().node(i);

		//if (n.name().find("nWeights") != std::string::npos) {
		//	vNames.push_back(n.name());
		//}
	//}

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
	static const std::string inputName = "Input/Conv2D:0"; // /kernel:0
	static const std::string outputName = "Output/bias:0";

	auto shape = tf::TensorShape({ BoardDepth, BoardSize, BoardSize});

	float ar[2527] = { 0.0 };

	//auto init = tf::Input::Initializer(*ar, shape);
	//auto placeH = tf::Placeholder

	tf::Tensor input(tf::DT_FLOAT, shape);
	std::copy_n(ar, 2527, input.flat<float>().data());

	/*
	auto inputMap = input.tensor<int, 3>();
	
	for (int z = 0; z < BoardDepth; ++z)
		for (int y = 0; y < BoardSize; ++y)
			for (int x = 0; x < BoardSize; ++x)
				inputMap(z, x, y) = ar[z * (y * BoardSize + x)];
	*/


	std::vector<std::pair<std::string, tf::Tensor>> inputs = {
		{ inputName, input },
	};

	std::vector<tf::Tensor> outputs;

	status = session->Run(inputs, { outputName }, {}, &outputs);


	tf::TTypes<float>::Flat flatOut = outputs[0].flat<float>();

	for (int i = 0; i < BoardSize2; ++i)
	{
		if (i % BoardSize == 0)
			std::cout << '\n';
		std::cout << std::setprecision(1) << std::setw(4) << flatOut(i) << ", ";
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
