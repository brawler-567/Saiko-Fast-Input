#pragma once

 

#include <functional>
#include <string>

class Regex {
private:
	class Data;
	Data* data;

public:
	Regex(const Regex& other);  
	Regex(Regex&& other);  
	Regex& operator=(const Regex& other);  
	Regex& operator=(Regex&& other);  

	Regex();
	Regex(const std::string& pattern);
	~Regex();

	std::string error() const;
	
	bool test(const std::string &str);
	void match(const std::string &str, bool global, std::function<void(const std::string &str, int match, int group)> func);
	std::string replace(const std::string &str, bool global, std::function<std::string(const std::string &str, int match, int group)> func);
};
