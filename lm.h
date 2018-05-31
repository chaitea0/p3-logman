//Log manager file

//todo resize vector correcty
//do you need an entry ID/
#include <algorithm>
#include <cctype>
#include <vector>
#include <unordered_map>
#include <numeric>
#include <functional>
#include <strings.h>
using namespace std;

//Struct to store individual log file info
struct lfile{
	lfile(string &mes_in, string &cat_in, uint32_t id_in):
	message(mes_in), cat(cat_in), entryID(id_in){}
	
	string message;
	string cat;
	uint32_t entryID;
};
struct lfile_comp{
	lfile_comp(vector<int64_t>& in):ts_list(in){}
	bool operator()(const lfile& one, const lfile& two){
		if (ts_list[one.entryID] == ts_list[two.entryID]){
			if (one.cat == two.cat)
				return one.entryID < two.entryID;
			else 
				return strcasecmp(one.cat.c_str(),two.cat.c_str()) < -1;
		}
		else 
			return ts_list[one.entryID] < ts_list[two.entryID];
	}
	vector<int64_t> &ts_list;
};

class lm{
public:
	unordered_map<string, vector<uint32_t>> cat_hash;
	unordered_map<string, vector<uint32_t>> mes_hash;
	vector<lfile> masterlist;
	vector<uint32_t> ex_list;
	vector<uint32_t> search_list;
	vector<int64_t> ts_list;
	vector<uint32_t> index_list;
	lfile_comp ml_func;
    bool searched;
    
	lm():ml_func(ts_list), searched(false){}

	void load_logfile(ifstream& logstream){
		string get_ts;
		string category;
		string message;
		uint32_t entryID = 0;
		int64_t timestamp;

		while(getline(logstream, get_ts, '|')){
			getline(logstream, category, '|');
			getline(logstream, message);

			timestamp = tsc_stl(get_ts);
			ts_list.push_back(timestamp);
			
			masterlist.emplace_back(message, category, entryID);
			entryID++;
		}

		sort(begin(masterlist), end(masterlist), ml_func);
		sort(begin(ts_list), end(ts_list));

		index_list.resize(masterlist.size());
		for(uint32_t i = 0; i < masterlist.size(); i++)
			index_list[masterlist[i].entryID] =  i;

		//Load words into unordered map
		for(uint32_t i = 0; i < masterlist.size(); i++){
			lfile &temp = masterlist[i];
			hash_map_load(temp.cat, temp.message, i);
		}
		
		cout << masterlist.size() << " entries read\n";
	}
	//Conversts a timestamp string to long form
	int64_t tsc_stl(const string &timestamp){
		int64_t temp = 0;
		temp += timestamp[0]-'0';
		temp = temp * 10 + timestamp[1] -'0';
		temp = temp * 10 + timestamp[3] -'0';
		temp = temp * 10 + timestamp[4] -'0';
		temp = temp * 10 + timestamp[6] -'0';
		temp = temp * 10 + timestamp[7] -'0';
		temp = temp * 10 + timestamp[9] -'0';
		temp = temp * 10 + timestamp[10]-'0';
		temp = temp * 10 + timestamp[12]-'0';
		temp = temp * 10 + timestamp[13]-'0';
		return temp;
	}
	// Loads keywords into category and message hashmaps
	void hash_map_load(string category, string message, uint32_t index){
		//Get all keywords
		vector<string> keywords;
		keyword_breakup(keywords, message);
		vector<string> cat_words;
		keyword_breakup(cat_words, category);
		
        cat_hash[category].push_back(index);
		for(size_t i = 0; i < keywords.size(); i++){
			vector<uint32_t> &temp = mes_hash[keywords[i]];
			if(temp.empty())
				temp.push_back(index);
			else if(temp.back() != index)
				temp.push_back(index);
        }
		for(size_t i = 0; i < cat_words.size(); i++){
			vector<uint32_t> &temp = mes_hash[cat_words[i]];
			if(temp.empty())
				temp.push_back(index);
			else if(temp.back() != index)
				temp.push_back(index);
		}

	}
	void keyword_breakup(vector<string> &keywords, string& input){
		auto lag = find_if(begin(input),end(input), ::isalnum);
		auto lead = lag;
        auto last = end(input);
		while(lag != last){
			while(lead != last && isalnum(*lead)){
                *lead = (char)tolower(*lead);
                lead++;
            }
			keywords.emplace_back(lag,lead);

			lag = find_if(lead,last, ::isalnum);
			lead = lag;
		}

	}
	void calculation(){
		char input;
		while(cin >> input){
			cout << "% ";
			switch (input) {
				//timestamps search
		        case 't':{
		        	search_list.erase(begin(search_list), end(search_list));
		            searched = true;
					string ts1;
                    string ts2;
			        getline(cin, ts1, ' ');
					getline(cin,ts1, '|');
                    getline(cin,ts2);
					if (ts1.length() != 14 || ts2.length() != 14)
						cerr << "Invalid timestamps\n";
                    else{
                        int64_t tstmp1 = tsc_stl(ts1);
                        int64_t tstmp2 = tsc_stl(ts2);
                        //Find lower bound of timestamps
                        auto first = begin(ts_list);
                        auto last = end(ts_list);
                        auto it1 = lower_bound(first, last, tstmp1);
                        auto it2 = upper_bound(first, last, tstmp2);
                        //while between range add
                        while(it1!=it2){
                        	search_list.push_back((uint32_t)(it1-first));
                        	it1++;
                        }
                        cout << "Timestamps search: " << search_list.size() << " entries found\n";
                    }
                    break;
		        }
		        //matching search(1 ts)
		        case 'm':{
		        	search_list.erase(begin(search_list), end(search_list));
		            searched = true;
					string ts;
			        getline(cin, ts, ' ');
					getline(cin,ts);
			        if(ts.length() != 14){
                        cerr << "Invalid timestamp length\n";
                    }
                    else{
                        int64_t tstmp1 = tsc_stl(ts);
                        //Find lower bound of timestamps
                        auto first = begin(ts_list);
                        auto last = end(ts_list);
                        auto it1 = lower_bound(first, last, tstmp1);
                        auto it2 = upper_bound(first, last, tstmp1);
                        //Add all equal timestamps
                        while(it1!=it2){
                        	search_list.push_back((uint32_t)(it1-first));
                        	it1++;
                        }
                        cout << "Timestamp search: " << search_list.size() << " entries found\n";
                    }
		            break;
		        }
		        //category serach
		        case 'c':{
		        	search_list.erase(begin(search_list), end(search_list));
		            searched = true;
					string cat;
			        getline(cin, cat, ' ');
					getline(cin,cat);
			    	transform(begin(cat), end(cat), begin(cat), ::tolower);

					auto it = cat_hash.find(cat);
			        if(it != end(cat_hash))
					    search_list.insert(end(search_list), begin(it->second), end(it->second));
					cout << "Category search: " << search_list.size() << " entries found\n";
		            break;
		        }
		        //keyword search
		        case 'k':{
		        	search_list.erase(begin(search_list), end(search_list));
		            searched = true;
					string keywords;
			        getline(cin, keywords, ' ');
					getline(cin,keywords);
					//stores indexes that contain all keywords
					vector<string> kws;
					keyword_breakup(kws, keywords);
					
					//initial vector of matches, set intersection
					auto it = mes_hash.find(kws[0]);
					if (it != end(mes_hash)){
						search_list.insert(end(search_list), begin(it->second), end(it->second));
                        for(size_t i = 1; i < kws.size(); i++){
                            it = mes_hash.find(kws[i]);
                            if (it != end(mes_hash)){
                                auto insec = set_intersection(begin(search_list), end(search_list), begin(it->second), end(it->second), begin(search_list));
                                search_list.erase(insec, end(search_list));
                            }
                            else{
                                search_list.erase(begin(search_list), end(search_list));
                                break;
                            }
                        }
                    }
					cout << "Keyword search: " << search_list.size() << " entries found\n";
		            break;
		        }
		        //append log entry
		        case 'a':{
		        	size_t num;
					cin >> num;
					if(num >= masterlist.size())
						cerr << "Invalid append, out of range\n";
                    else{
                        ex_list.push_back(index_list[num]);
                        cout << "log entry " << num << " appended\n";
                    }
		            break;
		        }
		        //append search entries
		        case 'r':{
		        	if (!searched)
						cerr << "No previous search\n";
					else{
						ex_list.insert(end(ex_list), begin(search_list), end(search_list));
						cout << search_list.size() << " log entries appended\n";
					}
		            break;
		        }
		        //delete log entry
		        case 'd':{
		        	size_t num;
					cin >> num;
					if(num >= ex_list.size())
						cerr << "Invalid excerpt list delete, out of range\n";
                    else{
                        auto it = begin(ex_list) + (int)num;
                        ex_list.erase(it);
                        cout << "Deleted excerpt list entry " << num << "\n";
                    }
		            break;
		        }
		        //move to beginning
		        case 'b':{
		        	size_t num;
					cin >> num;
					if(num >= ex_list.size())
						cerr << "Invalid excerpt list move to beginning, out of range\n";
                    else{
                        auto it = ex_list.rend()-(int)num-1;
                        rotate(it, it+1, ex_list.rend());
                        cout << "Moved excerpt list entry " << num << "\n";
                    }
		            break;
		        }
		        //move to end
		        case 'e':{
		        	size_t num;
					cin >> num;
					if(num >= ex_list.size())
						cerr << "Invalid excerpt list move to end, out of range\n";
                    else{
                        auto it = begin(ex_list)+(int)num;
                        rotate(it, it + 1, ex_list.end());
                        cout << "Moved excerpt list entry " << num << "\n";
                    }
		            break;
		        }
		        //sort excerpt list
		        case 's':{
		        	cout << "excerpt list sorted\n";
					if(ex_list.empty())
						cout << "(previously empty)\n";
					else{
						cout << "previous ordering:\n";
						cout << 0 << "|";
						print_logf(ex_list[0]);
						cout << "...\n";
						cout << ex_list.size()-1 << "|";
						print_logf(ex_list[ex_list.size()-1]);

						sort(begin(ex_list), end(ex_list));

						cout << "new ordering:\n";
						cout << 0 << "|";
						print_logf(ex_list[0]);
						cout << "...\n";
						cout << ex_list.size()-1 << "|";
						print_logf(ex_list[ex_list.size()-1]);
					}
		            break;
		        }
		        //clear excerpt list
		        case 'l':{
		        	cout << "excerpt list cleared\n";
					if(ex_list.empty())
						cout << "(previously empty)\n";
					else{
						cout << "previous contents:\n";
						
						cout << 0 << "|";
						print_logf(ex_list[0]);
						cout << "...\n";
						cout << ex_list.size()-1 << "|";
						print_logf(ex_list[ex_list.size()-1]);

						ex_list.erase(begin(ex_list), end(ex_list));
					}
		            break;
		        }
		        //print most recent search results
		        case 'g':{
		        	if (!searched)
			            cerr << "No previous search\n";
                    else{
                        for(uint32_t i = 0; i < search_list.size(); i++)
                            print_logf(search_list[i]);
                    }
		            break;
		        }
		        //print excerpt list
		        case 'p':{
		        	for(uint32_t i = 0; i < ex_list.size(); i++){
						cout << i << "|";
						print_logf(ex_list[i]);
					}
		            break;
		        }
		        //quit
		        case 'q':
		        	return;
		            break;
		        //ignore
		        case '#':{
		        	string trash;
		        	getline(cin, trash); 
		            break;
		        }
		        default:{
                    cerr << "Invalid Command\n";
                    string trash;
                    getline(cin, trash);
                }
		    }

		}
	}
	void print_logf(uint32_t index){
        int64_t tmp = ts_list[index];
		cout << masterlist[index].entryID << "|";
        cout << tmp / 1000000000;
        tmp = tmp % 1000000000;
        cout << tmp / 100000000;
        tmp = tmp % 100000000;
        cout << ":";
        cout << tmp / 10000000;
        tmp = tmp % 10000000;
        cout << tmp / 1000000;
        tmp = tmp % 1000000;
        cout << ":";
        cout << tmp / 100000;
        tmp = tmp % 100000;
        cout << tmp / 10000;
        tmp = tmp % 10000;
        cout << ":";
        cout << tmp / 1000;
        tmp = tmp % 1000;
        cout << tmp / 100;
        tmp = tmp % 100;
        cout << ":";
        cout << tmp / 10;
        tmp = tmp % 10;
        cout << tmp;
        cout << "|" << masterlist[index].cat << "|" << masterlist[index].message << "\n";
	}

private:

};
