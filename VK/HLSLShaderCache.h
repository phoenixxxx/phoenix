#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <filesystem>
#include <sstream>
#include <assert.h>
#include <Utils/Types.h>

#define MAX_COMPILE_CMD_LENGTH 4096

struct KeyValuePair
{
	std::string mKey;
	std::string mValue;
};

#define BUFSIZE 4096 
static bool ReadFromPipe(HANDLE handle)
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	bool error = false;

	do
	{
		PeekNamedPipe(handle, NULL, 0, NULL, &dwRead, NULL);
		if (dwRead)
		{
			ReadFile(handle, chBuf, BUFSIZE, &dwRead, NULL);
			WriteFile(hParentStdOut, chBuf, dwRead, &dwWritten, NULL);
			error = true;
		}
	} while (dwRead != 0);

	return error;
}

class HLSLShaderCache
{
public:
	bool Initialize(const stdstr_t& dxcExecutable, const stdstr_t& cachePath)
	{
		std::filesystem::path p(dxcExecutable);
		if (std::filesystem::exists(p) == false)
			return false;
		
		assert(!cachePath.empty());

		bool created = std::filesystem::create_directory(cachePath);

		mDxcExecutable = dxcExecutable;
		mCachePath = cachePath;

		return true;
	}
	std::string Compile(const char* file, const char* entryPoint, const char* shaderModel, const char* headerDir=nullptr)
	{
		std::vector<KeyValuePair> defines;
		return Compile(file,  entryPoint,  shaderModel, defines, headerDir);
	}
	std::string Compile(const char* file, const char* entryPoint, const char* shaderModel, std::vector<KeyValuePair>& defines, const char* headerDir=nullptr)
	{
		std::string defs;
		for (uint32_t iDef = 0; iDef < defines.size(); ++iDef)
		{
			defs += ("-D " + defines[iDef].mKey + "=" + defines[iDef].mValue + " ");
		}

		static char parameters[MAX_COMPILE_CMD_LENGTH];
		if(headerDir)
			sprintf_s(parameters, MAX_COMPILE_CMD_LENGTH, " -spirv -T %s -E %s %s %s -I %s", shaderModel, entryPoint, file, defs.c_str(), headerDir);
		else
			sprintf_s(parameters, MAX_COMPILE_CMD_LENGTH, " -spirv -T %s -E %s %s %s", shaderModel, entryPoint, file, defs.c_str());

		std::size_t hash = std::hash<std::string>{}(parameters);
		std::string filePath;
		bool error = true;
		for (const auto& entry : std::filesystem::directory_iterator(mCachePath))
		{
			auto stem = std::filesystem::path(entry).stem();
			std::stringstream convertor(stem.string());
			std::size_t hashFromFile;
			convertor >> hashFromFile;
			if (hash == hashFromFile)
			{
				//check timestamps
				auto hlslT = std::filesystem::last_write_time(file);
				auto compiledT = std::filesystem::last_write_time(entry.path().string());
				if (hlslT < compiledT)
				{
					filePath = entry.path().string();
				}
				break;
			}
		}
		if (filePath.empty())
		{
			HANDLE pipeRd = NULL;
			HANDLE pipeWr = NULL;

			SECURITY_ATTRIBUTES saAttr;
			saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
			saAttr.bInheritHandle = TRUE;
			saAttr.lpSecurityDescriptor = NULL;

			// Create a pipe for the child process's STDOUT. 
			bool pipeCreated = CreatePipe(&pipeRd, &pipeWr, &saAttr, 0);
			assert(pipeCreated);

			static char outputFile[MAX_COMPILE_CMD_LENGTH];
			sprintf_s(outputFile, MAX_COMPILE_CMD_LENGTH, "%s/%zu.spv", mCachePath.c_str(), hash);

			static char fullCmd[MAX_COMPILE_CMD_LENGTH]; 
			sprintf_s(fullCmd, MAX_COMPILE_CMD_LENGTH, "%s -Fo %s", parameters, outputFile);

			// Set up members of the PROCESS_INFORMATION structure. 
			PROCESS_INFORMATION processInfo;
			ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

			// Set up members of the STARTUPINFO structure. 
			// This structure specifies the STDIN and STDOUT handles for redirection.
			STARTUPINFOA info;
			ZeroMemory(&info, sizeof(STARTUPINFO));
			info.cb = sizeof(STARTUPINFO);
			info.hStdError = pipeWr;
			info.hStdOutput = pipeWr;
			info.hStdInput = 0;
			info.dwFlags |= STARTF_USESTDHANDLES;			
			
			if (CreateProcessA(
				mDxcExecutable.c_str(),
				fullCmd,
				NULL, NULL,
				TRUE,
				0, NULL, NULL, &info, &processInfo))
			{
				error = ReadFromPipe(pipeRd);

				WaitForSingleObject(processInfo.hProcess, INFINITE);
				CloseHandle(processInfo.hProcess);
				CloseHandle(processInfo.hThread);

				CloseHandle(pipeWr);
				CloseHandle(pipeRd);

				filePath = outputFile;
			}

		}

		if (error)
			return "";

		return filePath;
	}

private:
	std::string mDxcExecutable;
	std::string mCachePath;
};