/*
 * IniFile.hpp
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *   Modified: Andrey Novikov, 02 Feb 2018
 *    License: MIT
 */

#ifndef INIFILE_HPP_
#define INIFILE_HPP_

#include <map>
#include <vector>

#include <sstream>
#include <fstream>
#include <iostream>

#include <stdexcept>
#include <cassert>
#include <cmath>

#if defined(_WINDOWS)
	#include <string.h>
	#define  strcasecmp  _stricmp
#else
	#include <strings.h>
#endif

namespace ini
{
	/************************************
	 * OrderedMap
	 *
	 * Map which preserves the order of insertion
	 * Minimal subset of functionality
	 ************************************/
	template<typename K, typename T>  class OrderedMap {
		typedef  typename std::pair<K, T>                 Value;
		typedef  typename std::vector<Value>::size_type   Index;

		std::vector<Value> data_;  // data in insertion order
		std::map<K, Index> lookup_;  // lookup table

	public:
		typedef  typename std::vector<Value>::iterator    iterator;
		iterator begin(){  return data_.begin();  }
		iterator end(){  return data_.end();  }

		T& operator[](const K& k) {
			typename std::map<K, Index>::iterator p = lookup_.find(k);
			if( p != lookup_.end() ) {
				return data_[p->second].second;
			} else {
				data_.push_back( std::make_pair(k, T()) );
				lookup_[k] = data_.size() - 1;  // index of just added value
				return data_.back().second;
			}
		}

		std::size_t size() const {  return data_.size();  }

		bool has(const K& k){
			return lookup_.find(k) != lookup_.end();
		}

		void clear(){
			data_.clear();  lookup_.clear();
		}
	};

	/************************************
	 *          IniComment
	 ************************************/
	class IniComment
	{
		std::string s_;

		void trim_eol(){	// Cuts last '\n'
			if( s_.empty() )  return;
			const std::size_t n = s_.length() - 1;
			if( s_[n] == '\n' )
				s_.resize(n);
		}
	public:
		IniComment(){ ; }
		IniComment(const std::string& s) : s_(s) {
			trim_eol();
		}
		IniComment& operator=(const std::string& s){
			s_ = s;  trim_eol();
			return *this;
		}

		const std::string& get() const {  return s_;  }
		void output(std::ostream& os, char prefix) const {
			if( s_.empty() ) return;

			// Output prefixing each line with a comment char
			std::istringstream ss(s_);
			std::string line;
			while( std::getline(ss, line) )
				os << prefix << line << std::endl;
		}
	};


	/************************************
	 *          IniField
	 ************************************/
	class IniField
	{
	private:
		std::string value_;

	public:
		IniComment comment;

		IniField(){ ; }
		IniField(const std::string &value) : value_(value), comment() { ; }
		IniField(const IniField &f) : value_(f.value_), comment(f.comment) { ; }
		~IniField(){ ; }

		const std::string &asString() const {
			return value_;
		}

		int asInt() const {
			int n;
			if( ! (std::istringstream(value_) >> n) )
				throw std::domain_error("field is not an int");
			return n;
		}

		double asDouble() const {
			double x;
			if( ! (std::istringstream(value_) >> x) )
				throw std::domain_error("field is not a double");
			return x;
		}

		bool asBool() const {
			const int N = 4;
			const char* TT[N] = { "true", "yes", "y", "1" };
			const char* FF[N] = { "false", "no", "n", "0" };
			for( short i = 0; i < N; ++i ){
				if( strcasecmp(value_.c_str(), TT[i]) == 0 )
					return true;
				if( strcasecmp(value_.c_str(), FF[i]) == 0 )
					return false;
			}
			throw std::domain_error("field is not a bool");
		}

		IniField &operator=(const IniField &field) {
			value_ = field.value_;
			comment = field.comment;
			return *this;
		}

		IniField &operator=(const int value) {
			std::stringstream ss;   ss << value;
			value_ = ss.str();
			return *this;
		}

		IniField &operator=(const double value) {
			std::stringstream ss;   ss << value;
			value_ = ss.str();
			return *this;
		}

		IniField &operator=(const bool value){
			value_ = (value) ? "true" : "false";
			return *this;
		}

		IniField &operator=(const std::string &value){
			value_ = value;
			return *this;
		}
	};

	/************************************
	 *          IniSection
	 ************************************/
	class IniSection : public OrderedMap<std::string, IniField>
	{
	public:
		IniComment comment;

		IniSection() { ; }
	};


	/************************************
	 *          IniFile
	 ************************************/
	class IniFile : public OrderedMap<std::string, IniSection>
	{
	private:
		char fieldSep_;
		char commentChar_;

	public:
		IniFile(const char s = '=', const char c = ';')
			: fieldSep_(s), commentChar_(c) {
		}
		IniFile(const std::string &fn, const char s = '=', const char c = ';')
			: fieldSep_(s), commentChar_(c) {
			load(fn);
		}

		IniFile(std::istream &is, const char s = '=', const char c = ';')
			: fieldSep_(s), commentChar_(c) {
			decode(is);
		}

		void decode(std::istream &is)
		{
			clear();

			// Whitespace trimming
			struct {
				const char* WS;
				void left(std::string& s){
					s.erase(0, s.find_first_not_of(WS));
				}
				void right(std::string& s){
					s.erase(s.find_last_not_of(WS) + 1);
				}
				void operator()(std::string& s){
					left(s);   right(s);
				}
			} trim = { "\t \r" };

			int lineNo = 0;
			IniSection *cur_section = NULL;
			std::string cur_comment;

			// iterate file by line
			while( is.good() ) // !is.eof() && !is.fail()
			{
				std::string line;
				std::getline(is, line, '\n');   trim(line);
				++lineNo;

				if( line.empty() )  continue;

				// if line is a comment append to comment lines
				if(line[0] == commentChar_)
				{
					cur_comment += line.substr(1, line.npos) + "\n";
				}
				else if(line[0] == '[')
				{
					// line is a section
					// check if the section is also closed on same line
					std::size_t pos = line.find(']');
					if(pos == line.npos) {
						std::stringstream ss;
						ss << "l" << lineNo
						   << ": ini parsing failed, section not closed";
						throw std::logic_error(ss.str());
					}
					// check if the section name is empty
					if(pos == 1) {
						std::stringstream ss;
						ss << "l" << lineNo
						   << ": ini parsing failed, section name is empty";
						throw std::logic_error(ss.str());
					}
					// check if there is a newline following closing bracket
					if(pos + 1 != line.length()) {
						std::stringstream ss;
						ss << "l" << lineNo
						   << ": ini parsing failed, no end of line after section";
						throw std::logic_error(ss.str());
					}

					// retrieve section name
					std::string secName = line.substr(1, pos - 1);   trim(secName);
					cur_section = &((*this)[secName]);
					cur_section->comment = cur_comment;   cur_comment.clear();
				}
				else
				{
					// line is a field definition
					// check if section was already opened
					if(cur_section == NULL) {
						// field has no section -> create one with empty name
						cur_section = &((*this)[""]);
					}

					// find key value separator
					std::size_t pos = line.find(fieldSep_);
					if(pos == line.npos) {
						std::stringstream ss;
						ss << "l" << lineNo << ": ini parsing failed, no '=' found";
						throw std::logic_error(ss.str());
					}
					// retrieve field name and value
					std::string name = line.substr(0, pos);   trim.right(name);
					std::string value = line.substr(pos + 1, line.npos);   trim.left(value);

					IniField& field = (*cur_section)[name];
					field = value;
					field.comment = cur_comment;  cur_comment.clear();
				}
			}
		}

		void decode(const std::string &content) {
			std::istringstream ss(content);
			decode(ss);
		}

		void encode(std::ostream &os)
		{
			// iterate through all sections in this file
			for(IniFile::iterator it = this->begin(); it != this->end(); ++it) {
				if( it != begin() ) os << std::endl;
				it->second.comment.output(os, commentChar_);
				if( ! it->first.empty() )
					os << "[" << it->first << "]" << std::endl;

				// iterate through all fields in the section
				for(IniSection::iterator secIt = it->second.begin(); secIt != it->second.end(); ++secIt) {
					const IniField& field = secIt->second;
					field.comment.output(os, commentChar_);
					os << secIt->first << fieldSep_ << field.asString() << std::endl;
				}
			}
		}

		std::string encode(){
			std::ostringstream ss;
			encode(ss);
			return ss.str();
		}

		void load(const std::string &fileName) {
			std::ifstream is(fileName.c_str());
			decode(is);
		}

		void save(const std::string &fileName) {
			std::ofstream os(fileName.c_str());
			encode(os);
		}
	};
}

/**
 * Wrapper class for IniFile
 * Writes default value if key doesn't exist on reading
 */
class TIniAutoDefaults
{
	ini::IniFile _ini;
	std::string _file_name;

	ini::IniSection* _section;  // active section
	std::string _section_name;

	bool _updated;
	TIniAutoDefaults();

public:
	TIniAutoDefaults(const std::string& fn)
		: _ini(fn), _file_name(fn), _section(nullptr), _section_name(), _updated(false) {
		;
	}

	ini::IniFile& get_ini() {
		return _ini;
	}

	void save_if_updated() {
		if (_updated)
			_ini.save(_file_name);
	}

	void set_section(const char* s) {
		_section_name = s;
		_section = &(_ini[s]);
	}

	/**
	 * Read the key or create a new one with defaut value
	 *
	 * @param[in] key - key name in the active section
	 * @param[in] s0 - default value in case the key doesn't not exist
	 * @return    key value
	 */
	std::string read_string(const std::string& key, const std::string& s0) {
		assert(_section);
		ini::IniSection& sect = *_section;
		if (sect.has(key))
			return sect[key].asString();

		sect[key] = s0;   _updated = true;
		return s0;
	}

	int read_int(const std::string& key, int i0) {
		assert(_section);
		ini::IniSection& sect = *_section;
		try {
			if (sect.has(key))
				return sect[key].asInt();
		}
		catch (std::domain_error& e) {
			//hsLogWarning("%s", e.what());
            printf("%s", e.what());
		}

		sect[key] = i0;   _updated = true;
		return i0;
	}

	/**
	 * Reread the key, if updated assign the provided variable
	 *
	 * @param[in]     key  - key name in the active section
	 * @param[in/out] prevValue - saved previous value of the key
	 * @return        true if value was updated and false otherwise
	 */
	bool reread_int(const std::string& key, int& prevValue) {
		int val;
		try {
			if (!_section->has(key))
				return false;
			val = (*_section)[key].asInt();
		}
		catch (std::domain_error& e) {
			return false;
		}

		if (val == prevValue)
			return false;
        printf("  %s/%s: %d -> %d", _section_name.c_str(), key.c_str(), prevValue, val);
		//hsLogMessage("  %s/%s: %d -> %d", _section_name.c_str(), key.c_str(), prevValue, val);
		prevValue = val;

		return true;
	}

	double read_float(const std::string& key, double f0) {
		assert(_section);
		ini::IniSection& sect = *_section;
		try {
			if (sect.has(key))
				return sect[key].asDouble();
		}
		catch (std::domain_error& e) {
			//hsLogWarning("%s", e.what());
            printf("%s", e.what());
		}

		sect[key] = f0;   _updated = true;
		return f0;
	}

	/**
	 * Reread the key, if updated assign the provided variable
	 *
	 * @param[in]     key  - key name in the active section
	 * @param[in/out] prevValue - saved previous value of the key
	 * @return        true if value was updated and false otherwise
	 */
	bool reread_float(const std::string& key, double& prevValue) {
		double val;
		try {
			if (!_section->has(key))
				return false;
			val = (*_section)[key].asDouble();
		}
		catch (std::domain_error& e) {
			return false;
		}

		if (fabs(val - prevValue) < 1e-15)
			return false;

		//hsLogMessage("  %s/%s: %g -> %g", _section_name.c_str(), key.c_str(), prevValue, val);
        printf("  %s/%s: %g -> %g", _section_name.c_str(), key.c_str(), prevValue, val);
		prevValue = val;

		return true;
	}

	bool read_bool(const std::string& key, bool b0) {
		assert(_section);
		ini::IniSection& sect = *_section;
		try {
			if (sect.has(key))
				return sect[key].asBool();
		}
		catch (std::domain_error& e) {
			//hsLogWarning("%s", e.what());
            printf("%s", e.what());
		}

		sect[key] = b0;   _updated = true;
		return b0;
	}
};

#endif
