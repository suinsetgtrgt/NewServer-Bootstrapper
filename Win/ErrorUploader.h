#pragma once

#include <queue>
#include "rbx/boost.hpp"
#include "rbx/Thread.hpp"

#ifdef _WIN32 
// For mkdir
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include "_FindFirst.h"
#endif
#include "errno.h"

#include <memory>
#include <mutex>

class ErrorUploader
{
protected:
	struct data
	{
		std::queue<std::string> files;
		std::recursive_mutex sync;	// TODO: Would non-recursive be safe here?
		std::string url;
		int dmpFileCount;
		HANDLE hInterprocessMutex;

		data():dmpFileCount(0), hInterprocessMutex(nullptr) {}
	};
	std::shared_ptr<data> _data;
	std::unique_ptr<RBX::worker_thread> thread;

	static std::string MoveRelative(LPCTSTR fileName, std::string path);
public:
	ErrorUploader()
		:_data(std::make_shared<data>())
	{}
	void Cancel()
	{
		std::lock_guard<std::recursive_mutex> lock(_data->sync);
		while (!_data->files.empty())
			_data->files.pop();
	}
	bool IsUploading()
	{
		return !_data->files.empty();
	}
};

