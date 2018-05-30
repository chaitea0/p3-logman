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
	lfile(int64_t ts_in):message(""), cat(""), ts(ts_in){}
	lfile(string &mes_in, string &cat_in, int64_t ts_in):
	message(mes_in), cat(cat_in), ts(ts_in){}
	
	string message;
	string cat;
	int64_t ts;
};
//timestamp comparator
struct i_func{
	i_func(vector<lfile>& in):masterlist(in){}
	bool operator()(unsigned int ind1, unsigned ind2){
		const lfile& one = masterlist[ind1];
		const lfile& two = masterlist[ind2];
		return (one.ts == two.ts) ? ( (one.cat == two.cat) ? ind1 < ind2 : strcasecmp(one.cat.c_str(),two.cat.c_str())) : one.ts < two.ts;
	}
	vector<lfile> &masterlist;
};

class lm{
public:
	unordered_map<string, vector<int>> cat_hash;
	unordered_map<string, vector<int>> mes_hash;
	vector<lfile> masterlist;
	vector<int> ex_list;
	vector<int> search_list;
	vector<int> ts_sort;
	i_func index_func;
    bool searched;
	
	lm():index_func(masterlist), searched(false){}

	void load_logfile(ifstream& logstream){
		int32_t entryID = 0;
		//reserve to correct size?
		string get_ts;
		while(getline(logstream, get_ts, '|')){
			int64_t timestamp = tsc_stl(get_ts);
			string category;
			getline(logstream, category, '|');
			string message;
			getline(logstream, message);
			//Insert into masterlist
			masterlist.emplace_back(message, category, timestamp);
			entryID++;
		}
		
		ts_sort.resize(masterlist.size());
		iota(begin(ts_sort), end(ts_sort), 0);
		sort(begin(ts_sort), end(ts_sort), index_func);

		//Load words into unordered map
		for(size_t i = 0; i < ts_sort.size(); i++){
			lfile &temp = masterlist[(size_t)ts_sort[i]];
			hash_map_load(temp.cat, temp.message, ts_sort[i]);
		}
		
		cout << masterlist.size() << " entries read\n";
		//Last element used for sorting
		masterlist.emplace_back(0);
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
	void hash_map_load(string &category, string &message, int32_t entryID){
		//Load into category map
        string low_cat = category;
        transform(begin(low_cat), end(low_cat), begin(low_cat), ::tolower);
		cat_hash[low_cat].push_back((int)entryID);
		//Load all words into message map
		vector<string> keywords;
		keyword_breakup(keywords, message);
		vector<string> cat_words;
		keyword_breakup(cat_words, category);
		for(size_t i = 0; i < keywords.size(); i++){
			vector<int> &temp = mes_hash[keywords[i]];
            temp.push_back((int)entryID);
        }
		for(size_t i = 0; i < cat_words.size(); i++){
			vector<int> &temp = mes_hash[cat_words[i]];
			if(temp.empty())
				temp.push_back((int)entryID);
			else if(temp.back() != entryID)
				temp.push_back((int)entryID);    
		}

	}
	void keyword_breakup(vector<string> &keywords, string& input){
		auto lag = find_if(begin(input),end(input), ::isalnum);
		auto lead = lag;
		while(lag != end(input)){
			while(lead != end(input) && isalnum(*lead))
				lead++;
			
			string kw = string(lag, lead);
			transform(begin(kw), end(kw), begin(kw), ::tolower);
			keywords.push_back(kw);

			lag = find_if(lead,end(input), ::isalnum);
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
                        masterlist[masterlist.size()-1] = tstmp1-1;
                        auto it1 = lower_bound(begin(ts_sort), end(ts_sort), masterlist.size()-1, index_func);
                        masterlist[masterlist.size()-1] = tstmp2;
                        auto it2 = lower_bound(begin(ts_sort), end(ts_sort), masterlist.size()-1, index_func);
                        //while between range add
                        search_list.insert(end(search_list), it1, it2);
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
                        masterlist[masterlist.size()-1] = tstmp1-1;
                        auto it1 = lower_bound(begin(ts_sort), end(ts_sort), masterlist.size()-1, index_func);
                        masterlist[masterlist.size()-1] = tstmp1;
                        auto it2 = lower_bound(begin(ts_sort), end(ts_sort), masterlist.size()-1, index_func);
                        //Add all equal timestamps
                        search_list.insert(end(search_list), it1, it2);
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
			        if(it != cat_hash.end())
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
					if (it != end(mes_hash))
						search_list.insert(end(search_list), begin(it->second), end(it->second));
                    vector<int> temp1 = {1,2,3};
                    vector<int> temp2 = {};
					for(size_t i = 1; i < kws.size(); i++){
						if (search_list.empty())
							break;
						it = mes_hash.find(kws[i]);
                        
						if (it != end(mes_hash)){
							auto insec = set_intersection(begin(search_list), end(search_list), begin(it->second), end(it->second), begin(search_list), index_func);
                            search_list.erase(insec, end(search_list));
                        }
						else{
                            search_list.erase(begin(search_list), end(search_list));
							break;
                        }
					}
                    
					cout << "Keyword search: " << search_list.size() << " entries found\n";
		            break;
		        }
		        //append log entry
		        case 'a':{
		        	int num;
					cin >> num;
					if(num > (int)masterlist.size()-2)
						cerr << "Invalid append, out of range\n";
                    else{
                        ex_list.push_back(num);
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
		        	int num;
					cin >> num;
					if(num >= (int)ex_list.size())
						cerr << "Invalid excerpt list delete, out of range\n";
                    else{
                        auto it = begin(ex_list) + num;
                        ex_list.erase(it);
                        cout << "Deleted excerpt list entry " << num << "\n";
                    }
		            break;
		        }
		        //move to beginning
		        case 'b':{
		        	int num;
					cin >> num;
					if(num >= (int)ex_list.size())
						cerr << "Invalid excerpt list move to beginning, out of range\n";
                    else{
                        auto it = ex_list.rend()-num-1;
                        rotate(it, it+1, ex_list.rend());
                        cout << "Moved excerpt list entry " << num << "\n";
                    }
		            break;
		        }
		        //move to end
		        case 'e':{
		        	int num;
					cin >> num;
					if(num >= (int)ex_list.size())
						cerr << "Invalid excerpt list move to end, out of range\n";
                    else{
                        auto it = begin(ex_list)+num;
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
						print_logf((size_t)ex_list[0]);
						cout << "...\n";
						cout << ex_list.size()-1 << "|";
						print_logf((size_t)ex_list[ex_list.size()-1]);

						sort(begin(ex_list), end(ex_list), index_func);

						cout << "new ordering:\n";
						cout << 0 << "|";
						print_logf((size_t)ex_list[0]);
						cout << "...\n";
						cout << ex_list.size()-1 << "|";
						print_logf((size_t)ex_list[ex_list.size()-1]);
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
						print_logf((size_t)ex_list[0]);
						cout << "...\n";
						cout << ex_list.size()-1 << "|";
						print_logf((size_t)ex_list[ex_list.size()-1]);

						ex_list.erase(begin(ex_list), end(ex_list));
					}
		            break;
		        }
		        //print most recent search results
		        case 'g':{
		        	if (!searched)
			            cerr << "No previous search\n";
                    else{
                        for(unsigned int i = 0; i < search_list.size(); i++)
                            print_logf((size_t)search_list[i]);
                    }
		            break;
		        }
		        //print excerpt list
		        case 'p':{
		        	for(unsigned int i = 0; i < ex_list.size(); i++){
						cout << i << "|";
						print_logf((size_t)ex_list[i]);
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
	void print_logf(size_t index){
        int64_t tmp = masterlist[index].ts;
		cout << index << "|";
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
        cout << tmp / 1;
        cout << "|" << masterlist[index].cat << "|" << masterlist[index].message << "\n";
	}

private:

};
