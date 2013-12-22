namespace ns3 {

class ChronoMessages {
public:
	void Push(std::string name, int seq, std::string content);
	std::string Get(std::string name, int seq);


private:
	std::map<std::string, std::map<int, std::string> > m_messages;

};




}
