#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <torch/csrc/autograd/generated/variable_factories.h>
#include <torch/serialize/input-archive.h>
#include <vector>
#include <random>
#include <cassert>
#include <algorithm>
#include <torch/torch.h>
#include <cmath>
#include <cstdio>
#include "nn_modules.hpp"
#include "Agents/experience_replay.hpp"
#include "Agents/experience_replay_buffer.hpp"

namespace COMA_consts{ 
	const static size_t replay_buffer_size = 200*100; //1024*1024
	const float gamma = 0.99;
	//note: make sure tau mu is updated sufficenlty slower than tau_q
	const float tau_mu = 0.005; //Actor Learning Rate
	const float tau_q = 0.01; // Critic Learning Rate
	const float C = 10; // reset q_t every C steps
}

class COMAAgent{
	public:
		COMAAgent(size_t state_space, size_t action_space);
		~COMAAgent();

		static std::vector<float> evaluate_critic_NN(const std::vector<float>& s, const std::vector<float>& a);
		std::vector<float> evaluate_actor_NN(const std::vector<float>& s);
		static std::vector<float> evaluate_target_critic_NN(const std::vector<float>& s, const std::vector<float>& a);

		static void init_critic_NNs(size_t global_state_space, size_t global_action_space);

		// void updateTargetWeights();
		// void updateQCritic(const std::vector<float> Qvals, const std::vector<float> Qprime,bool verbose);
		// void updateMuActor(const std::vector<std::vector<float>> states, bool verbose);
		// void updateMuActorLink(std::vector<std::vector<float>> states,std::vector<std::vector<float>> all_actions,int agentNumber,bool withTime);

		[[maybe_unused]] void printAboutNN();

		static void set_batch_size(size_t i){batch_size = i;}
		static size_t get_batch_size() {return batch_size;}
		
	protected:
		inline static CriticNN qNN = CriticNN(1,1,1), qtNN = CriticNN(1,1,1); //TODO thread local
		ActorNN muNN;
		inline static size_t batch_size = 0;		

		// We need a global optimizer, not a new one in each step!!!!!!!!
		torch::optim::Adam optimizerMuNN;// = torch::optim::Adam(temp.parameters(),0.0001);
		torch::optim::Adam optimizerQNN;// = torch::optim::Adam(temp.parameters(),0.001);
};
