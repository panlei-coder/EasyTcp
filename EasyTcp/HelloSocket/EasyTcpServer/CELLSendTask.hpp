#ifndef _CELLSendTask_HPP
#define _CELLSendTask_HPP

#include<vector>
#include<thread>
#include<mutex>
#include<functional>

class CellSendTask{
	typedef std::function< void() > CellTask;
private:
	std::vector<CellTask> _cellSendTask;
	std::vector<CellTask> _cellSendTask_Buff;
	std::mutex _mutex;
public:
	CellSendTask()	{

	}

	~CellSendTask(){

	}

	void addCellTask(CellTask pTask){
		std::lock_guard<std::mutex> lg(_mutex);
		_cellSendTask_Buff.push_back(pTask);
	}

	void Start(){
		std::thread t(std::mem_fn(&CellSendTask::OnRun), this);
		t.detach();
	}

protected:
	void OnRun(){
		while (true){
			if (!_cellSendTask_Buff.empty()){
				std::lock_guard<std::mutex> lg(_mutex);
				for (auto pTask : _cellSendTask_Buff){
					_cellSendTask.push_back(pTask);
				}
				_cellSendTask_Buff.clear();
			}

			if (_cellSendTask.empty()){
				std::chrono::microseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			
			for (auto pTask : _cellSendTask){
				pTask();
			}
			_cellSendTask.clear();
		}
	}
};
#endif