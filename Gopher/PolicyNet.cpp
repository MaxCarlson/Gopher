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
	auto shape = tf::TensorShape({ BoardDepth, BoardSize, BoardSize});

	int ar[2527];

	auto init = tf::Input::Initializer(*ar, shape);
	//auto placeH = tf::Placeholder

	/*
	// and run the inference to your liking
	auto feedDict = ...
	auto outputOps = ...
	std::vector<tensorflow::Tensor> outputTensors;
	status = session->Run(feedDict, outputOps, {}, &outputTensors);

	*/
}









}
