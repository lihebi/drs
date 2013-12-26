/*
 * FILENAME: chrono-digest-tree.cc
 */
#include "chrono-digest-tree.h"
#include <algorithm>
#include "../pugixml.h"
#include "../hebi.h"
#include<openssl/evp.h>

namespace ns3 {

/*
 * Insert into m_state
 * UpdateRootDigest
 * return old digest
 * On Error: return ""
 */
std::string DigestTree::InsertAndUpdate(std::string name, int seq)
{
	if (m_state[name]>=seq) return "";
	std::string old = m_root_digest;
	m_state[name] = seq;
	UpdateRootDigest();
	return old;
}
int DigestTree::GetSeqByName(std::string name)
{
	return m_state[name];
}
/*
 * Format:
 * 	<li> <name> ... </name> <seq> ... </seq> </li>
 * 	<li> <name> ... </name> <seq> ... </seq> </li>
 */
std::string DigestTree::GetCurrentStateAsXML()
{
	pugi::xml_document doc;
	doc.load("");
	std::map<std::string, int>::const_iterator iter;
	for (iter=m_state.begin();iter!=m_state.end();iter++) {
		pugi::xml_node node = doc.append_child("li");
		node.append_child("name").text().set(iter->first.c_str());
		node.append_child("seq").text().set(std::to_string(iter->second).c_str());
	}
	hebi::xml_string_writer writer;
	doc.save(writer);
	return writer.result;
}
/*
 * Entry: xml (format as above)
 * Return: (name, seq) pairs to be fetch
 */
std::vector<boost::tuple<std::string, int> > DigestTree::CompareCurrentStateByXML(std::string xml)
{
	pugi::xml_document doc;
	doc.load(xml.c_str());
	pugi::xml_node node = doc.child("li");
	std::string name;
	int seq;
	std::vector<boost::tuple<std::string, int> > v;
	for (;node;node=node.next_sibling()) {
		name = node.child("name").text().get();
		seq = node.child("seq").text().as_int();
		for (int i=m_state[name]+1;i<=seq;i++) { //DEBUGED
			v.push_back(boost::make_tuple(name, i));
		}
	}
	return v;
}
std::string DigestTree::GetRootDigest()
{
	return m_root_digest;
}
void DigestTree::InitRootDigest()
{
	UpdateRootDigest();
}
void DigestTree::UpdateRootDigest()
{
	std::string s;
	std::map<std::string, int>::const_iterator iter;
	for (iter=m_state.begin();iter!=m_state.end();iter++) {
		s += iter->first + std::to_string(iter->second);
	}
	/* need a large function */
	m_root_digest = SHA256(s);
}
std::string DigestTree::SHA256(std::string s)
{
	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_len;
	OpenSSL_add_all_digests();
	md = EVP_get_digestbyname("sha256");
	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, md, NULL);
	EVP_DigestUpdate(mdctx, s.c_str(), s.size());
	EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	EVP_MD_CTX_destroy(mdctx);
	char buf[EVP_MAX_MD_SIZE];
	HashPrinter(buf, md_value, md_len);
	return std::string(buf);
}
void DigestTree::HashPrinter(char *dst_buf, unsigned char *src_buf, int size)
{
	for (int i=0;i<size;i++) {
		dst_buf += sprintf(dst_buf, "%02x", src_buf[i]);
	}
}


} //ns3
