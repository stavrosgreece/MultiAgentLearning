# multiagent_learning

This project Implements (MA)DDPG and ES on a warehouse domain using multiple
agent definitions with each learning algorithm, for purpose of comparing it to
[CCEA](https://github.com/JenJenChung/multiagent_learning).

This purpose implements the following algorithms on the warehouse domain/framework
 - ES
 - (MA)DDPG [Disabled] (use `-DENABLE_DDPG` to enable)


## Installation ##

### Linux ###

Install system dependencies (on ubuntu):
```
sudo apt install libboost-dev libyaml-cpp-dev
```

In the working folder of your choice, clone the project code
```
git clone git@github.com:stavrosgreece/multiagentlearning.git
```


install CTorch/LibTorch (C++ version of PyTorch):
```
cd multiagentlearning
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-latest%2Bcpu.zip
unzip libtorch-cxx11-abi-shared-with-deps-latest+cpu.zip
rm libtorch-cxx11-abi-shared-with-deps-latest+cpu.zip
```
Note: you can download and install LibTorch with different computer platform (e.g. CUDA/RocM) on https://pytorch.org/, be sure to instal the c++11+ ABI version



Build the code:
```
cd multiagentlearning
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. && make
```

## Running preconfigured projects

Run the project configured by `config.yaml` using six threads:
```
./testWarehouse -c ../config.yaml -t 6
```

## Debug
```
cmake -DCMAKE_BUILD_TYPE=Debug .. && make && gdb --args ./testWarehouse -c ../config.yaml
```
```
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && make && valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./testWarehouse -c ../config.yaml
```
## Profiling
With Perf (Recommended if you are not made out RAM)
```
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && make && perf record -g ./testWarehouse -c ../config.yaml
```
With Callgrind
```
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && make && valgrind --tool=callgrind ./testWarehouse -c ../config.yaml
```

# Authors
 - [Kallinteris Andreas](https://github.com/kallinteris-andreas)
 - [Starvos Orfanoudakis](https://github.com/stavrosgreece/)
 
This is repo is a fork of https://github.com/JenJenChung/multiagent_learning
