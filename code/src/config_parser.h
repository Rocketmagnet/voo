#ifndef CONFIG_PARSER_H_INCLUDED
#define CONFIG_PARSER_H_INCLUDED

#include <vector>
#include <string>

using namespace std;

struct ConfigDatum
{
    ConfigDatum()
    {
    }
    
    ConfigDatum(string n, string v)
    : name(n),
      value(v)
    {
    }
    
    string name;
    string value;
};


class ConfigParser
{
public:
    ConfigParser(const string &fn);
    ConfigParser() {}
            
    void Write();
    
    int      Exists(const string &name);
    double   GetDouble(  const string &name);
    int      GetInt(     const string &name);
    int      GetIntWithDefault(const string& name, int def);
    string   GetString(  const string &name);

    void     SetDouble(  const string &name,       double    value);
    void     SetInt(     const string &name,       int       value);
    void     SetString(  const string &name, const string   &value);
    
    string DoubleToString(double v);
    
private:
    int GetIndexFromName(const string &name) const;
    
    vector<ConfigDatum>     configData;
    string                  fileName;
};


#endif // CONFIG_PARSER_H_INCLUDED

