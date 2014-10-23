#ifndef REXECUTE_H
#define REXECUTE_H

int execute(const std::vector< std::vector<std::string> >& parsed);

std::vector<char**> c_compatible(const std::vector< std::vector<std::string> >& parsed);

void c_delete(const std::vector< std::vector<std::string> >& parsed,
		std::vector<char**>& c_parsed);

#endif
