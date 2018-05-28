//Log manager file

//todo resize vector correcty
//do you need an entry ID/
#include <algorithm>
#include <cctype>
#include <vector>
#include <unordered_map>
#include <numeric>
using namespace std;

//Struct to store individual log file info
struct lfile{
	lfile(int64_t ts_in):message(""), cat(""), ts(ts_in), ID(0){}
	lfile(string &mes_in, string &cat_in, int64_t ts_in, int32_t ID_in):
	message(mes_in), cat(cat_in), ts(ts_in), ID(ID_in){}
	
	string message;
	string cat;
	int64_t ts;
	int32_t ID;
};
//master comparator
struct m_func{
	bool operator()(const lfile& one, const lfile& two){
		return (one.ts == two.ts) ? ( (one.cat == two.cat) ? one.ID < two.ID : strcasecmp(one.cat.c_str(),two.cat.c_str())) : one.ts < two.ts;
	}
};
//timestamp comparator
struct i_func{
	i_func(vector<lfile>& in):masterlist(in){}
	bool operator()(unsigned int ind1, unsigned ind2){
		const lfile& one = masterlist[ind1];
		const lfile& two = masterlist[ind2];
		return (one.ts == two.ts) ? ( (one.cat == two.cat) ? one.ID < two.ID : strcasecmp(one.cat.c_str(),two.cat.c_str())) : one.ts < two.ts;
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
	m_func mast_c;
	i_func index_func;

	//https://stackoverflow.com/questions/3072795/how-to-count-lines-of-a-file-in-c
	lm(): index_func(masterlist){}

	void load_logfile(ifstream& logstream){
		int32_t entryID = 0;
		//reserve to correct size?
		string line;
		while(getline(logstream, line)){
			//Get timestamp
			int64_t timestamp = tsc_stl(line);
			//Get second | divider
			size_t div_2 = line.find('|', 15);
			string category = line.substr(15, div_2-15);
			string message = line.substr(div_2+1, line.length());
			//Load words into unordered map
			hash_map_load(category, message, entryID);
			//Insert into masterlist
			masterlist.emplace_back(message, category, timestamp, entryID);
			entryID++;
		}
		
		ts_sort.resize(masterlist.size());
		iota(begin(ts_sort), end(ts_sort), 0);
		sort(begin(ts_sort), end(ts_sort), index_func);
		
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
		auto it = cat_hash.find(category);
		if (it == cat_hash.end())
			cat_hash.insert(make_pair(category, vector<int>(1,(int)entryID)));
		else
			it->second.push_back((int)entryID);
		//Load all words into message map
		int lag = 0;
		int lead = 0;
		lead = find_delim(message, lag);
		while(lead != -1){
			string message_kw = message.substr((size_t)lag, (size_t)lead-1);
			it = mes_hash.find(message_kw);
			if (it == mes_hash.end())
				mes_hash.insert(make_pair(message_kw, vector<int>(1,(int)entryID)));
			else
				it->second.push_back((int)entryID);
			
			lag = lead+1;
			lead = find_delim(message, lag);
		}

	}
	void calculation(){
		char input;
		while(true){
			cout << "% ";
			cin >> input;
			switch (input) {
				//timestamp search
		        case 't':
		        	tss_t();
		            break;
		        //matching search(1 ts)
		        case 'm':
		        	tss_m();
		            break;
		        //category serach
		        case 'c':
		        	cs_c();
		            break;
		        //keyword search
		        case 'k':
		        	ks_k();
		            break;
		        //append log entry
		        case 'a':
		        	al_a();
		            break;
		        //append search entries
		        case 'r':
		        	asr_r();
		            break;
		        //delete log entry
		        case 'd':
		        	dle_d();
		            break;
		        //move to beginning
		        case 'b':
		        	m2b_b();
		            break;
		        //move to end
		        case 'e':
		        	m2e_e();
		            break;
		        //sort excerpt list
		        case 's':
		        	sel_s();
		            break;
		        //clear excerpt list
		        case 'l':
		        	cel_l();
		            break;
		        //print most recent search results
		        case 'g':
		        	psr_g();
		            break;
		        //print excerpt list
		        case 'p':
		        	pel_p();
		            break;
		        //quit
		        case 'q':
		        	return;
		            break;
		        //ignore
		        case '#':
		        	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
		            break;
		        default:
		        	cerr << "Invalid Command\n";
		    }

		}
	}
	//Timestamps search
	void tss_t(){
		search_list.erase(begin(search_list), end(search_list));
		
		string ts1;
		string ts2;
		cin >> ts1;
		cin >> ts2;
		if (ts1.length() != 14 || ts2.length()!= 14)
			cerr << "Invalid timestamps\n";
		int64_t tstmp1 = tsc_stl(ts1);
		int64_t tstmp2 = tsc_stl(ts2);
		//Find lower bound of timestamps
		masterlist[masterlist.size()-1] = tstmp1;
		auto it = lower_bound(begin(ts_sort), end(ts_sort), masterlist.size()-1, index_func);
		//while between range add
		while(masterlist[(size_t)*it].ts <= tstmp2)
			search_list.push_back(*it++);
		cout << "Timestamps search: " << search_list.size() << " entries found\n";
	}
	//Timestamp search
	void tss_m(){
		search_list.erase(begin(search_list), end(search_list));
		
		string ts;
		cin >> ts;
		int64_t tstmp1 = tsc_stl(ts);
		//Find lower bound of timestamps
		masterlist[masterlist.size()-1] = tstmp1;
		auto it = lower_bound(begin(ts_sort), end(ts_sort), masterlist.size()-1, index_func);
		//Add all equal timestamps
		while(masterlist[(size_t)*it].ts == tstmp1)
			search_list.push_back(*it++);
		cout << "Timestamp search: " << search_list.size() << " entries found\n";
	}
	//Category search
	void cs_c(){
		search_list.erase(begin(search_list), end(search_list));
		
		string cat;
		cin >> cat;

		auto it = cat_hash.find(cat);
		search_list.insert(end(search_list), begin(it->second), end(it->second));
		cout << "Category search: " << search_list.size() << " entries found\n";
	}
	//Keywords search
	void ks_k(){
		search_list.erase(begin(search_list), end(search_list));
		
		string keywords;
		getline(cin,keywords);
		//stores indexes that contain all keywords
		vector<int> all_kw;
		int lag = 0;
		int lead = 0;
		//find if catgory or messages match
		lead = find_delim(keywords, lag);
		auto it1 = cat_hash.find(keywords.substr((size_t)lag, (size_t)lead-1));
		auto it2 = mes_hash.find(keywords.substr((size_t)lag, (size_t)lead-1));
		//initial vector of matches, set intersection
		all_kw.insert(end(all_kw), begin(it1->second), end(it1->second));
		set_intersection(begin(all_kw), end(all_kw), begin(it2->second), end(it2->second), all_kw.begin());

		while(lead != -1){
			it1 = cat_hash.find(keywords.substr((size_t)lag, (size_t)lead-1));
			it2 = mes_hash.find(keywords.substr((size_t)lag, (size_t)lead-1));
			set_intersection(begin(all_kw), end(all_kw), begin(it1->second), end(it1->second), all_kw.begin());
			set_intersection(begin(all_kw), end(all_kw), begin(it2->second), end(it2->second), all_kw.begin());
			lag = lead+1;
			lead = find_delim(keywords, lag);
		}
		cout << "Keyword search: " << search_list.size() << " entries found\n";
	}
	//Helper function for separating strings into keywords
	int find_delim(string &temp, int i){
		size_t k = (size_t)i;
		for(; k < temp.length(); k++)
			if (!isalnum(temp[k]))
				return (int)k;
		return -1;
	}
	//Append
	void al_a(){
		int num;
		cin >> num;
		if(num >= (int)masterlist.size()-2){
			cerr << "Invalid append, out of range\n";
			return;
		}
		ex_list.push_back(num);
		cout << "log entry " << num << " appended\n";
	}
	//append search results
	void asr_r(){
		if (search_list.empty())
			cerr << "No previous search\n";
		else{
			//sort search entires
			sort(begin(ex_list), end(ex_list), index_func);
			ex_list.insert(end(ex_list), begin(search_list), end(search_list));
			cout << search_list.size() << " log entries appended\n";
		}

	}
	//delete excerpt list delete
	void dle_d(){
		int num;
		cin >> num;
		if(num >= (int)ex_list.size()){
			cerr << "Invalid excerpt list delete, out of range\n";
			return;
		}
		auto it = begin(ex_list) + num;
		ex_list.erase(it);

		//delete excerpt list entry
		cout << "Deleted excerpt list entry " << num << "\n";
	}
	//move to beginning
	void m2b_b(){
		int num;
		cin >> num;
		if(num >= (int)ex_list.size()){
			cerr << "Invalid excerpt list move to beginning, out of range\n";
			return;
		}
		auto it = begin(ex_list)+num;
		rotate(begin(ex_list), it, it + 1);
		cout << "Moved excerpt list entry " << num << "\n";
	}
	//move to end
	void m2e_e(){
		int num;
		cin >> num;
		if(num >= (int)ex_list.size()){
			cerr << "Invalid excerpt list move to end, out of range\n";
			return;
		}
		auto it = begin(ex_list)+num;
		rotate(it, it + 1, ex_list.end());
		cout << "Moved excerpt list entry " << num << "\n";
	}
	//sort excerpt list
	void sel_s(){
		cout << "Excerpt list sorted\n";
		if(ex_list.empty())
			cout << "(previously empty)\n" << endl;
		else{
			cout << "previous ordering\n" << endl;
			cout << 0 << "|";
			print_logf(0);
			cout << "...\n" << endl;
			cout << ex_list.size()-1 << "|";
			print_logf(ex_list.size()-1);

			sort(begin(ex_list), end(ex_list), index_func);

			cout << "new ordering\n" << endl;
			cout << 0 << "|";
			print_logf(0);
			cout << "...\n" << endl;
			cout << ex_list.size()-1 << "|";
			print_logf(ex_list.size()-1);
		}
	}
	//clear excert list
	void cel_l(){
		cout << "Excerpt list cleared\n";
		if(ex_list.empty())
			cout << "(previously empty)\n" << endl; 
		else{
			cout << "previous contents\n" << endl;
			
			cout << 0 << "|";
			print_logf(0);
			cout << "...\n" << endl;
			cout << ex_list.size()-1 << "|";
			print_logf(ex_list.size()-1);

			ex_list.erase(begin(ex_list), end(ex_list));
		}
	}
	//print search results
	void psr_g(){
		for(unsigned int i = 0; i < search_list.size(); i++)
			print_logf((size_t)search_list[i]);
	}
	//print excerpt list
	void pel_p(){
		for(unsigned int i = 0; i < ex_list.size(); i++){
			cout << i << "|";
			print_logf((size_t)ex_list[i]);
		}
	}
	void print_logf(size_t index){
		lfile& tmp = masterlist[index];
		cout << tmp.ID << "|"
			 << tmp.ts / 1000000000
		     << tmp.ts / 100000000
		     << ":"
		     << tmp.ts / 10000000
		     << tmp.ts / 1000000
		     << ":"
		     << tmp.ts / 100000
		     << tmp.ts / 10000
		     << ":"
		     << tmp.ts / 1000
		     << tmp.ts / 100
		     << ":"
		     << tmp.ts / 10
		     << tmp.ts / 1
			 << "|" << tmp.cat << "|" << tmp.message << "\n";
	}

private:

};
