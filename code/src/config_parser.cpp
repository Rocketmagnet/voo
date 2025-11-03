#include "config_parser.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <cctype>


class BadConversion : public std::runtime_error {
public:
  BadConversion(const std::string& s)
    : std::runtime_error(s)
    { }
};

inline double ConvertToDouble(const std::string& s)
{
  std::istringstream i(s);
  double x;
  if (!(i >> x))
    throw BadConversion("convertToDouble(\"" + s + "\")");
  return x;
}

inline int ConvertToInt(const std::string& s)
{
  std::istringstream i(s);
  int x;
  if (!(i >> x))
    throw BadConversion("convertToInt(\"" + s + "\")");
  return x;
}

// trim from start
static inline std::string &ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

void ConfigParser::LoadConfigFile(const string& fn)
{
    ifstream source;                    // build a read-Stream
    source.open(fn, ios::in);     // open data
    fileName = fn;

    if (source.is_open())
    {
        while (!source.eof())
        {
            ConfigDatum configDatum;
            string line;

            getline(source, line);

            size_t eqPos = line.find("=");

            if (eqPos > 0)
            {
                configDatum.name  = line.substr(0, eqPos - 1);
                configDatum.value = line.substr(eqPos + 1);

                ltrim(rtrim(configDatum.name));
                ltrim(rtrim(configDatum.value));
                configData.push_back(configDatum);
            }

        }
        initialised = true;
    }

    source.close();
}


ConfigParser::ConfigParser(const string &fn)
: fileName(fn),
  initialised(false)
{    
    LoadConfigFile(fn);
}

int ConfigParser::GetIndexFromName(const string &name) const
{
    size_t i, n = configData.size();

    for (i=0; i<n; i++)
    {
        if (configData[i].name == name)
        {
            return i;
        }
    }

    return -1;
}

double   ConfigParser::GetDouble(const string &name)
{
    int i = GetIndexFromName(name);

    if (i>=0)
    {
        return ConvertToDouble(configData[i].value);
    }
    else
    {
        return 0.0;
    }
}

int ConfigParser::GetInt(const string &name)
{
    int i = GetIndexFromName(name);

    if (i>=0)
    {
            return ConvertToInt(configData[i].value);
    }
    else
    {
        return 0;
    }
}

int ConfigParser::GetIntWithDefault(const string& name, int def)
{
    int i = GetIndexFromName(name);

    if (i >= 0)
    {
        return ConvertToInt(configData[i].value);
    }
    else
    {
        return def;
    }
}

string ConfigParser::GetString(const string &name)
{
    int i = GetIndexFromName(name);

    if (i>=0)
    {
        return configData[i].value;
    }
    else
    {
        return string("");
    }
}



int ConfigParser::Exists(const string &name)
{
    int i = GetIndexFromName(name);
    
    return (i >= 0);
}

    
void     ConfigParser::SetDouble(const string &name, double value)
{
    string v = std::to_string(value);
    SetString(name, v);
}

void     ConfigParser::SetInt(const string &name, int value)
{
    string v = std::to_string(value);
    SetString(name, v);    
}

void     ConfigParser::SetString(const string &name, const string &value)
{
    size_t i, n = configData.size();

    for (i=0; i<n; i++)
    {
        if (configData[i].name == name)
        {
            configData[i].value = value;
            return;
        }
    }
    
    configData.push_back(ConfigDatum(name, value));    
}

void ConfigParser::Write()
{
    ofstream source;                    // build a read-Stream

    //cout << "ConfigParser::Write()" << fileName << endl;

    source.open(fileName, ios::out);     // open data
    
    if (source.is_open())
    {
        size_t i, n = configData.size();

        for (i=0; i<n; i++)
        {
            //cout   << "  " << configData[i].name << " = " << configData[i].value << endl;
            source <<         configData[i].name << " = " << configData[i].value << endl;
        }
    
        source.close();
    }
}

string ConfigParser::DoubleToString(double v)
{
    string str = std::to_string(v);
    
    if (str.find(".") >= 0)
        str.erase ( str.find_last_not_of('0') + 1, std::string::npos );
    
    if (str.back() == '.')
        str.pop_back();
    
    return str;
}
