#include "constraints.h"
#include <regex>
using namespace std;

int g_next_constraint_tag = 0;

bool constraint_set::add_constraint(const char* constraint_str) {

	constraint* c = 0;

	// First make sure we have a constraint...
	cmatch matches;
	regex r("^\\s*constraint\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S*)[\r\n]?$", regex::icase);

	bool result = regex_match(constraint_str, matches, r);
	if (result == false) {
		_cprintf("Could not parse constraint string %s\n", constraint_str);
		return false;
	}

	if (matches.size() < 5) {
		_cprintf("Illegal constraint string %s\n", constraint_str);
	}

	string str_constraint_type(matches[1].first, matches[1].second);
	string str_vertices(matches[2].first, matches[2].second);
	string str_start(matches[3].first, matches[3].second);
	string str_end(matches[4].first, matches[4].second);
	string str_arguments;
	if (matches.size() == 6) {
		str_arguments.assign(matches[5].first, matches[5].second);
	}

	// Parse start and end times
	float start_time;
	result = sscanf(str_start.c_str(), "%f", &start_time);
	if (!result) {
		_cprintf("Could not parse start time in constraint %s\n", constraint_str);
		return false;
	}

	float end_time;
	result = sscanf(str_end.c_str(), "%f", &end_time);
	if (!result) {
		_cprintf("Could not parse end time in constraint %s\n", constraint_str);
		return false;
	}

	bool all_vertices = false;
	std::vector<unsigned int> vertices;
	if (str_vertices == "all") {
		all_vertices = true;
	}
	else {
		regex r("\\s*\\(?([^\\)]*)\\)?\\s*$");
		str_vertices = regex_replace(str_vertices,
			r,
			"$1");

		regex delimiter("[ ,]");
		sregex_token_iterator i(str_vertices.begin(), str_vertices.end(), delimiter, -1);

		// Defaults to "end"
		sregex_token_iterator j;

		while (i != j) {
			int val;
			string s(*i);

			// Is this an x-y formatted string?
			regex r("^\\s*(\\d+)\\s*-\\s*(\\d+)[\r\n]?$", regex::icase);

			result = regex_match(s.c_str(), matches, r);
			if (result == true) {
				string start(matches[1].first, matches[1].second);
				string end(matches[2].first, matches[2].second);

				int istart, iend;
				result = sscanf(start.c_str(), "%d", &istart);
				if (result == 0) {
					_cprintf("Error converting range %s\n", s.c_str());
					++i; continue;
				}

				result = sscanf(end.c_str(), "%d", &iend);
				if (result == 0) {
					_cprintf("Error converting range %s\n", s.c_str());
					++i; continue;
				}

				if (end < start) {
					_cprintf("Illegal range %s\n", s.c_str());
				}

				_cprintf("Converted a range: %d - %d\n", istart, iend);

				for (int k = istart; k <= iend; k++)
					vertices.push_back(k);

			}
			else {
				result = sscanf(s.c_str(), "%d", &val);
				if (result == 0) {
					_cprintf("Error converting vertex %s\n", s.c_str());
					++i; continue;
				}
				vertices.push_back(val);
			}
			++i;
		} // for each token in the vertex list
	}

	int type = CONSTRAINT_UNINITIALIZED;

	if (str_constraint_type == "FORCE_CONSTANT") {
		type = CONSTRAINT_FORCE_CONSTANT;
		constraint_force_constant* fc;
		c = fc = new constraint_force_constant;

		// parse force vector
		regex r("\\s*\\(?([^\\)]*)\\)?\\s*$");

		bool result = regex_match(str_arguments.c_str(), matches, r);
		if (result == false || matches.size() != 2) {
			_cprintf("Could not parse force string %s\n", str_arguments.c_str());
			delete c;
			return false;
		}

		string str_forcevector(matches[1].first, matches[1].second);
		result = fc->force.set(str_forcevector);
		if (result == false) {
			_cprintf("Error converting vector from %s\n", str_arguments.c_str());
		}
	}

	else if (str_constraint_type == "POSITION_CONSTANT") {
		type = CONSTRAINT_POSITION_CONSTANT;
		constraint_position_constant* pc;
		c = pc = new constraint_position_constant;

		// parse position vector
		regex r("\\s*\\(?([^\\)]*)\\)?\\s*$");

		bool result = regex_match(str_arguments.c_str(), matches, r);
		if (result == false || matches.size() != 2) {
			_cprintf("Could not parse position string %s\n", str_arguments.c_str());
			delete c;
			return false;
		}

		string str_posvector(matches[1].first, matches[1].second);

		// See if this is a "lock" command
		if (strcmp("lock", str_posvector.c_str()) == 0) {
			pc->lock = true;
			pc->position.set(0, 0, 0);
		}

		// Look for a position vector
		else {
			result = pc->position.set(str_posvector);
			if (result == false) {
				_cprintf("Error converting vector from %s\n", str_arguments.c_str());
				delete pc;
				return false;
			}
		}

	}

	else {
		_cprintf("Unhandled constraint type: %s\n", str_constraint_type.c_str());
		return false;
	}

	if (c) {
		c->start_time = start_time;
		c->end_time = end_time;
		c->affects_all_vertices = all_vertices;
		c->vertices.swap(vertices);
		// c->vertices = vertices;
		c->type = type;
		_cprintf("Assigning type %d to c %x (%d)\n", type, c, c->vertices.size());
		c->tag = g_next_constraint_tag++;
		add_constraint(c);
	}

	return true;

}


void constraint_set::add_constraint(constraint* c) {

	constraints_by_start_time.insert(c);
	constraints_by_end_time.insert(c);

}

void constraint_set::clear() {
	active_constraints.clear();
	constraints_by_start_time.clear();
	constraints_by_end_time.clear();
}


void constraint_set::reset() {
	active_constraints.clear();
	next_constraint_start = constraints_by_start_time.begin();
	next_constraint_end = constraints_by_end_time.begin();
}


constraint* constraint_set::get_next_constraint_start(float curtime) {

	// Are there any more starting constraints?
	if (constraints_by_start_time.empty() || next_constraint_start == constraints_by_start_time.end()) {
		return 0;
	}
	constraint* c = *next_constraint_start;
	if (c->start_time <= curtime) {

		// We need to activate this constraint
		active_constraints[c->tag] = c;

		next_constraint_start++;
		return c;
	}
	return 0;

}


constraint* constraint_set::get_next_constraint_end(float curtime) {

	// Are there any more ending constraints?
	if (constraints_by_end_time.empty() || next_constraint_end == constraints_by_end_time.end()) {
		return 0;
	}
	constraint* c = *next_constraint_end;
	if (c->end_time <= curtime) {

		// We need to deactivate this constraint
		std::map<unsigned int, constraint*>::iterator iter;
		iter = active_constraints.find(c->tag);

		if (iter == active_constraints.end()) {
			_cprintf("Oops... constraint %d is ending but not active\n", c->tag);
		}
		else {
			active_constraints.erase(iter);
		}

		next_constraint_end++;
		return c;
	}
	return 0;
}

constraint_set::~constraint_set() {

	// Clean out our list of constraints
	std::multiset<constraint*, lt_constraint_start>::iterator iter =
		constraints_by_start_time.begin();

	while (iter != constraints_by_start_time.end()) {
		delete (*iter);
		iter++;
	}

}

void constraint_set::idle(float curtime) {

	constraint* c;
	do { c = get_next_constraint_start(curtime); } while (c != 0);
	do { c = get_next_constraint_end(curtime); } while (c != 0);

}