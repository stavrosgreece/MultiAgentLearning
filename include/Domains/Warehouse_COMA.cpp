#include "Warehouse_COMA.hpp"

Warehouse_COMA::~Warehouse_COMA(void){
	delete whGraph;
	whGraph = 0;
	for (size_t i = 0; i < whAGVs.size(); i++){
		delete whAGVs[i];
		whAGVs[i] = 0;
	}
	for (size_t i = 0; i < whAgents.size(); i++){
		delete whAgents[i];
		whAgents[i] = 0;
	}	
}


epoch_results Warehouse_COMA::simulate_epoch_COMA (bool verbose){
	epoch_results results; // TODO fix
	//std::normal_distribution<float> n_process(1, N_proc_std_dev);
	//std::default_random_engine n_generator(time(NULL));
	assert(COMAAgent::get_batch_size());	

	///SIMULATE step
	std::vector<experience_replay> replay; //empty buffer
	replay.reserve(nSteps*COMAAgent::get_batch_size());

	for (size_t e = 0; e != COMAAgent::get_batch_size(); e++){
		InitialiseNewEpoch();

		for (size_t t = 0; t < nSteps; t++){
			const std::vector<float> cur_state(N_EDGES*(incorporates_time+1),0);
			std::vector<float> actions = query_actor_MATeam(cur_state);

			traverse_one_step(actions);
			 
			// Log Perfomance Counters
			size_t totalMove = 0, totalEnter = 0, totalWait = 0, totalSuccess = 0, totalCommand = 0;
			for (size_t k = 0; k < whAGVs.size(); k++){
				totalMove += whAGVs[k]->GetMoveTime();
				totalEnter += whAGVs[k]->GetEnterTime();
				totalWait += whAGVs[k]->GetWaitTime();
				totalSuccess += whAGVs[k]->GetNumCompleted();
				totalCommand += whAGVs[k]->GetNumCommanded();
			}
			for (size_t i = 0; i < whAGVs.size(); i++)// Reset all AGVs
				whAGVs[i]->ResetPerformanceCounters();
			if(verbose)
				std::cout<<"Stats:\n Total Move+Enter: "<<totalMove+totalEnter<<
					"Total wait: "<<totalWait<< " Total Success: "<<totalSuccess<<std::endl;
			results.update((float) totalSuccess/COMAAgent::get_batch_size(), (float) totalMove/COMAAgent::get_batch_size(), (float) totalEnter/COMAAgent::get_batch_size(), (float) totalWait/COMAAgent::get_batch_size());

			float reward = totalMove + totalEnter;
			const std::vector<float> next_state = get_edge_utilization();

			replay.push_back({cur_state, next_state, actions, reward});			
		}
	}
	std::cout<<"Start Training"<<std::endl;

	std::vector<float> q_input_states, q_input_actions, rewardsV;
	q_input_states.reserve(COMAAgent::get_batch_size()*nSteps);
	q_input_actions.reserve(COMAAgent::get_batch_size()*nSteps);
	rewardsV.reserve(COMAAgent::get_batch_size()*nSteps);

	for (size_t i = 0; i < maTeam.size(); ++i){
		q_input_states.clear();
		q_input_actions.clear();
		rewardsV.clear();

		for (size_t b = 0; b < COMAAgent::get_batch_size(); ++b){
			for (size_t t = 0; t < nSteps; ++t){
				// q_input[b][t][0] = replay[b*nSteps + t].action[i];
				q_input_actions.push_back(replay[b*nSteps + t].action[i]);
				// q_input[b][t][1] = replay[b*nSteps + t].current_state[i];
				q_input_states.push_back(replay[b*nSteps + t].current_state[i]);
				//q_input_states.insert(q_input_states.end(), replay[b*nSteps + t].current_state.begin(), replay[b*nSteps + t].current_state.end());
				// std::cout<<replay[b*nSteps + t].current_state[i]<<std::endl;
				rewardsV.push_back(replay[b*nSteps + t].reward);
			}
		}

		//Train Critic 
		torch::Tensor Q_targets = COMAAgent::evaluate_target_critic_NN(q_input_states,q_input_actions).squeeze(1);
		torch::Tensor Q = COMAAgent::evaluate_critic_NN(q_input_states,q_input_actions).squeeze(1);

		torch::Tensor rewards = torch::tensor(rewardsV);//.unsqueeze(0);
		//std::cout<<rewards<<std::endl;
		//std::cout << Q_targets << std::endl;
		Q_targets = Q_targets+rewards;

		torch::Tensor dQ = Q_targets - Q;
		torch::Tensor critic_loss = torch::mean(torch::pow(dQ,2));
		// std::cout<<critic_loss<<std::endl;

		COMAAgent::optimizerQNN->zero_grad();
		critic_loss.backward();
		COMAAgent::optimizerQNN->step();


	}


	const torch::Tensor monte_carlo_samples = torch::rand(COMA_consts::actor_samples);
	//TODO try sampling from history

	for (size_t i = 0; i < maTeam.size(); ++i) {
		//Train Actor
		//const torch Tensor =



		//torch::Tensor Baseline = ;

		// for (int b = 0; b < COMAAgent::get_batch_size(); ++b){
		// 	for (int t = 0; t < nSteps; ++t){
		// 		targets[b][t][i] = temp[b*nSteps + t]; //Targets y_t for each agent
		// 	}
		// }
	}


	// if( COMA_consts::C )
	COMAAgent::reset_target_critic(); // Reset Target Critic In EVERY epoch(might change)

	return results;
}



void Warehouse_COMA::InitialiseMATeam(){
	assert(whAgents.size());//this must be called after whAgents have been initialized
	if (algo != algo_type::coma){
		std::cout << "ERROR: Invalid agent_defintion" << std::endl;
		exit(EXIT_FAILURE);
	}

	COMAAgent::init_critic_NNs(N_EDGES*(1+incorporates_time), N_EDGES);
	assert(maTeam.empty());
	if (agent_type == agent_def::centralized){
		std::cout << "Centralized Agent is not supported with COMA" << std::endl;
		exit(EXIT_FAILURE);
	}
	else if (agent_type == agent_def::link)
		for (size_t i = 0; i < whGraph->GetEdges().size(); i++)
			maTeam.push_back(new COMAAgent((1+incorporates_time), 1));
	else if (agent_type == agent_def::intersection){
		std::cout << "Intersection Agent Does not work with COMA" << std::endl;
		exit(EXIT_FAILURE);
	}

	assert(!maTeam.empty());
}

std::vector<float> Warehouse_COMA::query_actor_MATeam(std::vector<float> states){
	assert(states.size() == N_EDGES*(1 + incorporates_time));
	assert(agent_type == agent_def::link);
	std::vector<float> actions;
	actions.reserve(N_EDGES);

	for (size_t i = 0; i < maTeam.size(); i++)
		if(incorporates_time)
			actions.push_back(maTeam[i]->evaluate_actor_NN({states[i], states[i+N_EDGES]})[0]);
		else{
			float t = (maTeam[i]->evaluate_actor_NN({states[i]}))[0];
			actions.push_back(t);
		}
	return actions;
}