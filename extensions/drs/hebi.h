namespace hebi {


struct xml_string_writer : pugi::xml_writer
{
	std::string result;
	virtual void write(const void *data, size_t size) {
		result += std::string(static_cast<const char*>(data), size);
	}
};
void StringSplit(std::string &s, char indent);
void StringGet(const std::string &s, char indent, int seq);




} //hebi
