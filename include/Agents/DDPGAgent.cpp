#include "DDPGAgent.h"


DDPGAgent::DDPGAgent(size_t state_space, size_t action_space){
	//Create NNs
	q_criticNN = new NeuralNet(state_space + action_space,
		1, (state_space + action_space) * 2);
	q_target_criticNN = new NeuralNet(state_space + action_space,
		1, (state_space + action_space) * 2);	
	mu_actorNN = new NeuralNet(action_space, action_space, action_space*2);
	mu_target_actorNN = new NeuralNet(action_space, action_space, action_space*2);

	assert(q_criticNN->GetNumIn() == action_space + state_space);
	assert(q_target_criticNN->GetNumIn() == action_space + state_space);

	//TODO Randomize weights of NNs
	//for (int i = 0; i != q_target_criticNN.GetWeightsA().size(); i++)
		//q_target_criticNN->GetWeightsA().(i) = ((double) rand());
	//for (int i = 0; i != q_target_criticNN.size(); i++)
		//assert(q_target_criticNN(i) => 0 && q_target_criticNN(i) <= 1);

	q_target_criticNN->SetWeights(q_criticNN->GetWeightsA(),
		q_criticNN->GetWeightsB());
	mu_target_actorNN->SetWeights(mu_actorNN->GetWeightsA(),
		mu_actorNN->GetWeightsB());

	replay_buffer.reserve(REPLAY_BUFFER_SIZE);
}

DDPGAgent::~DDPGAgent(){
	delete(q_criticNN);
	delete(q_target_criticNN);
	delete(mu_actorNN);
	delete(mu_target_actorNN);
	q_criticNN = NULL;
	q_target_criticNN = NULL;
	mu_actorNN = NULL;
	mu_target_actorNN = NULL;
}


void DDPGAgent::ResetEpochEvals(){
	
}

VectorXd DDPGAgent::EvaluateActorNN_DDPG(VectorXd s){
	return mu_actorNN->EvaluateNN(s);
}

VectorXd DDPGAgent::EvaluateCriticNN_DDPG(VectorXd s,VectorXd a){	
	VectorXd input(s.size() + a.size());
	input << s, a;

	return q_criticNN->EvaluateNN(input);
}

VectorXd DDPGAgent::EvaluateTargetActorNN_DDPG(VectorXd s){	
	return mu_target_actorNN->EvaluateNN(s);
}

VectorXd DDPGAgent::EvaluateTargetCriticNN_DDPG(VectorXd s,VectorXd a){
	VectorXd input(s.size() + a.size()),output;
	input << s, a;	
	assert(input.size() == s.size()+a.size());
	for (int i = 0; i != s.size(); i++)
		assert(input[i] == s[i]);
	for (int i = 0; i != a.size(); i++)
		assert(input[i+s.size()] == a[i]);
	output = q_target_criticNN->EvaluateNN(input);	
	return output;
}	

void DDPGAgent::addToReplayBuffer(replay r){
	assert(r.next_state.size() == r.current_state.size() && r.current_state.size() == 38);
	
	if(replay_buffer.size() < REPLAY_BUFFER_SIZE){
		replay_buffer.push_back(r);	
	}else{
		replay_buffer[rand()%REPLAY_BUFFER_SIZE] = r;
	}		
}

vector<replay> DDPGAgent::getReplayBufferBatch(size_t size){
	std::vector<replay> temp;
	assert(replay_buffer.size() >= size);

	for (int i = 0; i != size; i++){
		int r = rand()%size;
		temp.push_back(replay_buffer[r]);
		replay_buffer.erase(replay_buffer.begin()+r);
	}
	assert(temp.size() == size );	
	for (size_t i = 0; i < size ; i++){
		replay_buffer.push_back(temp[i]);
	}	

	assert(temp.size() == size);
	return temp;
	//TODO get random minibatch
}

void DDPGAgent::updateTargetWeights(){

	MatrixXd QtA = TAU*q_criticNN->GetWeightsA() + (1-TAU)*q_target_criticNN->GetWeightsA();
	MatrixXd QtB = TAU*q_criticNN->GetWeightsB() + (1-TAU)*q_target_criticNN->GetWeightsB();
	q_target_criticNN->SetWeights(QtA,QtB);

	MatrixXd MutA = TAU*mu_actorNN->GetWeightsA() + (1-TAU)*mu_target_actorNN->GetWeightsA();
	MatrixXd MutB = TAU*mu_actorNN->GetWeightsB() + (1-TAU)*mu_target_actorNN->GetWeightsB();
	mu_target_actorNN->SetWeights(MutA,MutB);
}