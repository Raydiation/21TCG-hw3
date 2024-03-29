/**
 * Framework for 2048 & 2048-like Games (C++ 11)
 * agent.h: Define the behavior of variants of agents including players and environments
 *
 * Author: Theory of Computer Games (TCG 2021)
 *         Computer Games and Intelligence (CGI) Lab, NYCU, Taiwan
 *         https://cgilab.nctu.edu.tw/
 */

#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include "weight.h"
#include <fstream>

const int MAX_INDEX = 23;// the max tile index could occur.
const int tuple_number = 32;
const int tuple_length = 6;//4*6
const long map_size = powl(MAX_INDEX, tuple_length);
const float MIN_FLOAT = -std::numeric_limits<float>::max();
const float epsilon = 1e-5;
const float lembda = 0.5;

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

	const static std::vector<std::vector<int> > pattern;

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

/**
 * base agent for agents with randomness
 */
class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};

/**
 * base agent for agents with weight tables and a learning rate
 */
class weight_agent : public agent {
public:
	weight_agent(const std::string& args = "") : agent(args), alpha(0) {
		if (meta.find("init") != meta.end())
			init_weights(meta["init"]);
		if (meta.find("load") != meta.end())
			load_weights(meta["load"]);
		if (meta.find("alpha") != meta.end())
			alpha = float(meta["alpha"]);
	}
	virtual ~weight_agent() {
		if (meta.find("save") != meta.end())
			save_weights(meta["save"]);
	}

protected:
	virtual void init_weights(const std::string& info) {
	    net.emplace_back(map_size);
	    for(int i1=0;i1<MAX_INDEX;i1++){
			for(int i2=0;i2<MAX_INDEX;i2++){
				for(int i3=0;i3<MAX_INDEX;i3++){
					for(int i4=0;i4<MAX_INDEX;i4++){
						for(int i5=0;i5<MAX_INDEX;i5++){
							for(int i6=20;i6<MAX_INDEX;i6++){
								net[0][((((i1*MAX_INDEX+i2)*MAX_INDEX+i3)*MAX_INDEX+i4)*MAX_INDEX+i5)*MAX_INDEX+i6] = 5000;
								net[0][((((i1*MAX_INDEX+i2)*MAX_INDEX+i3)*MAX_INDEX+i4)*MAX_INDEX+i6)*MAX_INDEX+i5] = 5000;
								net[0][((((i1*MAX_INDEX+i2)*MAX_INDEX+i3)*MAX_INDEX+i6)*MAX_INDEX+i5)*MAX_INDEX+i4] = 5000;
								net[0][((((i1*MAX_INDEX+i2)*MAX_INDEX+i6)*MAX_INDEX+i4)*MAX_INDEX+i5)*MAX_INDEX+i3] = 5000;
								net[0][((((i1*MAX_INDEX+i6)*MAX_INDEX+i3)*MAX_INDEX+i4)*MAX_INDEX+i5)*MAX_INDEX+i2] = 5000;
								net[0][((((i6*MAX_INDEX+i2)*MAX_INDEX+i3)*MAX_INDEX+i4)*MAX_INDEX+i5)*MAX_INDEX+i1] = 5000;
							}
						}
					}
				}
			}
		}
      	net_A.emplace_back(map_size);
      	for(int i = 0; i < net_A[0].size(); i++)
      		net_A[0][i] = epsilon;

      	for(int i = 1; i < 4; i++)
      	{
      		net_A.emplace_back(1);
	      	net_A[i] = net_A[0];
	  	}
		for(int i = 1; i < 4; i++)
      	{
          	net.emplace_back(1); // create an empty weight table with size map_size
         	net[i] = net[0];
	 	}
		for(int i = 0; i < 4; i++)
		{
			net_E.emplace_back(1);
			net_E[i] = net_A[0];
		}
	}
	virtual void load_weights(const std::string& path) {
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (!in.is_open()) std::exit(-1);
		uint32_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		net.resize(size / 3);
        net_A.resize(size / 3);
        net_E.resize(size / 3);
		for (weight& w : net) in >> w;
		for (weight& w : net_A) in >> w;
		for (weight& w : net_E) in >> w;
		in.close();
	}
	virtual void save_weights(const std::string& path) {
		std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		uint32_t size = net.size() + net_A.size() + net_E.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : net) out << w;
		for (weight& w : net_A) out << w;
		for (weight& w : net_E) out << w;
		out.close();
	}

protected:
	std::vector<weight> net;
	std::vector<weight> net_E;
	std::vector<weight> net_A;
    float alpha;
};

/**
 * random environment
 * add a new random tile to an empty cell
 * 2-tile: 90%
 * 4-tile: 10%
 */
class rndenv : public random_agent {
public:
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
		space({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }), popup(0, 9) {}

	virtual action take_action(const board& after) {
		std::shuffle(space.begin(), space.end(), engine);
		for (int pos : space) {
			if (after(pos) != 0) continue;
			board::cell tile = popup(engine) ? 1 : 2;
			return action::place(pos, tile);
		}
		return action();
	}

private:
	std::array<int, 16> space;
	std::uniform_int_distribution<int> popup;
};

/**
 * dummy player
 * select a legal action randomly
 */
class player : public weight_agent {
public:
	player(const std::string& args = "") : weight_agent("name=dummy role=player " + args),
		opcode({ 0, 1, 2, 3 }), space({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }) {}

	unsigned long get_feature(const board& boardstate, const std::vector<int>& pattern)
    {
        unsigned long encode = 0;
        int tile;
        for(int i : pattern)
        {
            encode *= MAX_INDEX;
            tile = (boardstate[i / 4][i % 4] > MAX_INDEX - 1) ? MAX_INDEX - 1 : boardstate[i / 4][i % 4];
            encode += (unsigned long)tile;
        }
        return encode;
    }
    float board_value(const board& boardstate)
    {
        float value = 0;
        for(int i = 0; i < tuple_number; i++)
        {
       		value += net[i/8][get_feature(boardstate, pattern[i])];
        }
        return value;
    }

    /**********************2-ply modify*********************/
	virtual action take_action(const board& before) {//2-ply
		int best_op = 0;
		int best_reward = 0;
		float best_expectation = MIN_FLOAT;
		board best_afterstate;
		for(int op : opcode)
        {
            board after = before;
            int reward = after.slide(op);
            if(reward == -1) continue;

            float expectation = put_tile(after, 1);
            if(expectation + reward > best_expectation + best_reward)
            {
                best_op = op;
                best_reward = reward;
                best_afterstate = after;
                best_expectation = expectation;
            }
        }
        if(best_expectation != MIN_FLOAT)
            history.push_back({best_afterstate, best_reward});
        return action::slide(best_op);
	}
	float put_tile(const board& before, const int& depth)
	{
		if(depth == 0) return board_value(before);
	    float expectation = 0;
	    float empty_grid = 0;
        for (int pos : space)
        {
			if (before(pos) != 0) continue;
			empty_grid += 1;

			for(int tile : {1, 2})
			{
				board after = before;
				after(pos) = tile;
				{
				    if(tile == 1)
                        expectation += move_simulation(after, depth) * 0.9;
                    else
                        expectation += move_simulation(after, depth) * 0.1;
				}
			}
		}
		expectation /= empty_grid;
		return expectation;
	}
	float move_simulation(const board& before, const int& depth) {
		float expectation;
		float best_expectation = MIN_FLOAT;
		bool change = 0;
		for(int op : opcode)
        {
            board after = before;
            int reward = after.slide(op);
            if(reward == -1) continue;

            expectation = put_tile(after, depth - 1);
            if(expectation + reward > best_expectation)
            {
                best_expectation = expectation + reward;
                change = 1;
            }
        }
        return change ? best_expectation : 0;
	}
	/**********************2-ply modify*********************/

	virtual void close_episode(const std::string& flag = "")
	{
		float history_value = 0;
    	train_weights(history[history.size()-1].afterstate, history_value);//T-1 turn
    	for(int i = history.size() - 2; i >= 0; i--)
    	{
    		train_weights(history[i].afterstate, history[i+1].afterstate, history[i+1].reward, history_value);
    	}
    	history.clear();
    	return;
	}

	void train_weights(const board& prev_board,const board& next_board,const float &reward, float &history_value)
	{
	    float delta = reward + board_value(next_board) - board_value(prev_board);
	   	history_value = alpha * delta + history_value * lembda;
	    for(int i = 0; i < tuple_number; i++)
        {
            long feature = get_feature(prev_board, pattern[i]);
            float learning_rate = fabs(net_E[i/8][feature]) / net_A[i/8][feature];
        	net[i/8][get_feature(prev_board, pattern[i])] += history_value * learning_rate / 8;
        	net_E[i/8][feature] += delta / 8;
        	net_A[i/8][feature] += fabs(delta) / 8;
        }
        return;
	}
	void train_weights(const board& final_board, float& history_value)
	{
        float delta = board_value(final_board);//TD target is 0;
        history_value = -alpha * delta;
        for(int i = 0; i < tuple_number; i++)
        {
            long feature = get_feature(final_board, pattern[i]);
            float learning_rate = fabs(net_E[i/8][feature]) / net_A[i/8][feature];
        	net[i/8][get_feature(final_board, pattern[i])] += history_value * learning_rate / 8;
        	net_E[i/8][feature] += delta / 8;
        	net_A[i/8][feature] += fabs(delta) / 8;
        }
        return;
	}
	struct state{
		board afterstate;
		int reward;
	};
	std::vector<state> history;
private:
	std::array<int, 4> opcode;
	std::array<int, 16> space;
};

const std::vector<std::vector<int>> agent::pattern = {
	{3,2,1,0,4,5},
	{0,4,8,12,13,9},
	{12,13,14,15,11,10},
	{15,11,7,3,2,6},
	{0,1,2,3,7,6},
	{12,8,4,0,1,5},
	{15,14,13,12,8,9},
	{3,7,11,15,14,10},//outter six type

	{7,6,5,4,8,9},
	{4,5,6,7,11,10},
	{11,10,9,8,4,5},
	{8,9,10,11,7,6},
	{13,9,5,1,2,6},
	{1,5,9,13,14,10},
	{14,10,6,2,1,5},
	{2,6,10,14,13,9},//inner six type

	{0,1,5,9,8,4},
	{0,4,5,6,2,1},
	{3,7,6,5,1,2},
	{3,2,6,10,11,7},
	{12,13,9,5,4,8},
	{12,8,9,10,14,13},
	{15,11,10,9,13,14},
	{15,14,10,6,7,11},//outter 2*3 rectangle

	{1,2,6,10,9,5},
	{2,1,5,9,10,6},
	{8,4,5,6,10,9},
	{4,8,9,10,6,5},
	{7,11,10,9,5,6},
	{11,7,6,5,9,10},
	{14,13,9,5,6,10},
	{13,14,10,6,5,9}//inner 2*3 rectangle
};

