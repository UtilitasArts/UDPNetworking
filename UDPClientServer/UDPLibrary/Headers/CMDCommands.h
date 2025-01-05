#pragma once

#include <filesystem>
namespace fs = std::filesystem;

namespace CMD {
	template <typename T, typename... Args>
	inline std::string MultiCMD(T first, Args&... rest)	{
		std::string CombinedCMD = first;
		((CombinedCMD += " && " + rest), ...);
		return CombinedCMD;
	}
	inline std::string Command(std::string command, std::string vars = "")	{
		return command + vars;
	}
	inline std::string Terminal(std::string command, std::string vars = "/K") {
		return "start cmd " + vars +  "\"" + command + "\"";
	}
	inline std::string SetPath(std::filesystem::path path) {
		return "cd \"" + path.string() + "\"";
	}
	inline std::string SetString(std::string string) {
		return "\"" + string + "\"";
	}
}

