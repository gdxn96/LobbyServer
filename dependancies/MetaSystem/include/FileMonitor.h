#pragma once
#include <unordered_map>
#include <sys/stat.h>
#include <time.h>
#include <functional>

//allows the monitoring of files for changes and the invocation of a callback when it does
class FileMonitor
{
public:

	static FileMonitor& getInstance()
	{
		static FileMonitor& instance = FileMonitor();
		return instance;
	}

	//register file for monitoring
	void WatchFile(std::string filePath, std::function<void()> callback)
	{
		m_monitoredFiles[filePath] = std::make_pair(getTimeInfo(filePath.c_str()), callback);
	}

	void MonitorFiles(float dt)
	{
		m_fileCheckDelay += dt;
		if (m_fileCheckDelay >= MAX_DELAY)
		{
			for (auto& val : m_monitoredFiles)
			{
				tm _timeInfo = getTimeInfo(val.first.c_str());

				if (isOutOfDate(val.second.first, _timeInfo))
				{
					val.second.second();
					val.second.first = _timeInfo;
				}
			}
		}
	}

	tm getTimeInfo(const char* path)
	{
		struct stat _result;
		if (stat(path, &_result) == 0)
		{
			tm _timeInfo = tm();
			localtime_s(&_timeInfo, &_result.st_mtime);
			return _timeInfo;
		}
	}

	inline bool isOutOfDate(tm td1, tm td2)
	{
		if (td1.tm_sec != td2.tm_sec ||
			td1.tm_min != td2.tm_min ||
			td1.tm_hour != td2.tm_hour)
		{
			return true;
		}
		else
		{
			return false;
		}
	}


private:
	float m_fileCheckDelay;
	const float MAX_DELAY;
	std::unordered_map<std::string, std::pair<tm, std::function<void()>>> m_monitoredFiles;

	FileMonitor() : MAX_DELAY(5) {};
};

