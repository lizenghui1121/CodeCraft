#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <set>
//#include <list>
#include <deque>
#include <iterator>
#include <unordered_map>
#include <sstream>
//#include<ctime>
//#include <sys/mman.h>
//#include <fcntl.h>
//#include <unistd.h>

using namespace std;


typedef unsigned int  ui;

//#define TEST

class CycleFinder
{
public:
	int max_cycle_length = 7;
	int min_cycle_length = 2;
	vector<set<int>> graph;  // 图后继
	vector<set<int>> graph_pre;  // 图前驱
	vector<int> in_degree;
	unordered_map<unsigned int, int> id_map; // 存储id的映射
	vector<unsigned int> idx; // 从小到大 id 的顺序
	vector<unsigned int> data_inputs; // 输入流
	map<unsigned int, vector<vector<unsigned int>>> cycles_group_by_length; // 分组存储结果
	vector<vector <int>> result;
	vector<vector <unsigned int>> path_by_length;
	vector<unordered_map<int, set<int>>> p3;

	int edge_count = 0;
	int node_count = 0;
	int avg_degree = 0;
	int cycle_count = 0; // 环的总数目

	// 深度遍历 6+1
	vector<int> visit;
	vector<int> visit1;


	/*void load_data(string &input_file_path)
	{
		FILE* file;
		fopen(&file, input_file_path.c_str(), "r");
		unsigned int from, to, amount;
		while (fscanf(file, "%u,%u,%u", &from, &to, &amount) != EOF) {
			data_inputs.push_back(from);
			data_inputs.push_back(to);
			edge_count++;
		}
		
#ifdef TEST
		printf("%d edges in Total\n", edge_count);
#endif
	}*/

	void parseInput(string& input_file_path) {
		FILE* file = fopen(input_file_path.c_str(), "r");
		unsigned int from, to, amount;;
		while (fscanf(file, "%u,%u,%u", &from, &to, &amount) != EOF) {
			data_inputs.push_back(from);
			data_inputs.push_back(to);
			edge_count++;
		}
#ifdef TEST
		printf("%d edges in Total\n", edge_count);
#endif
	}

	/*void parse_input_by_mmap()
	{
		const char *input_file_path = "/data/test_data.txt";
		int fd = open(input_file_path, O_RDWR | O_CREAT, 0666);
		long file_size = lseek(fd, 0, SEEK_END);
		char* mmapped_data = (char *)mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
		int data = 0;
		for (int i = 0; i < file_size; ++i) {
			if ('0' <= mmapped_data[i] && mmapped_data[i] <= '9') {
				data = (data << 1) + (data << 3) + (mmapped_data[i] & 0xf);
			} else if(mmapped_data[i] ==','){
				data_inputs.push_back(data);
				edge_count++;
				data = 0;
			}
			else {
				data = 0;
			}
		}
		edge_count = edge_count >> 1;
#ifdef TEST
		cout << "file size: " << file_size << endl;
#endif
	}*/

	void init_graph()
	{
		vector<unsigned int> temp = data_inputs;
		sort(temp.begin(), temp.end());
		temp.erase(unique(temp.begin(), temp.end()), temp.end());
		idx = temp;

		for (unsigned int raw_id : temp) {
			id_map[raw_id] = node_count++;
		}
		avg_degree = edge_count / node_count;
#ifdef TEST
		cout << node_count << " nodes in total\n";
		cout << avg_degree <<" avg degree " << endl;
#endif
		int input_length = data_inputs.size();
		graph = vector<set<int>>(node_count);
		graph_pre = vector<set<int>>(node_count);
		in_degree = vector<int>(node_count, 0);
		int u, v;
		for (int i = 0; i < input_length; i += 2) {
			u = id_map[data_inputs[i]], v = id_map[data_inputs[i + 1]];
			graph[u].insert(v);
			graph_pre[v].insert(u);
			++in_degree[v];
		}
	}

	void topo_sort()
	{
		int nodes = node_count;
		deque<int> quene;
		vector<int> delete_quene;
		for (int i = 0; i < nodes; ++i) {
			if (in_degree[i] == 0) {
				quene.push_back(i);
			}
		}
		while (!quene.empty()) {
			int u = quene.front();
			quene.pop_front();
			delete_quene.push_back(u);
			for (int v : graph[u]) {
				if (--in_degree[v] == 0) {
					quene.push_back(v);
				}
			}
		}
		for (int j : delete_quene) {
			graph[j].clear();
		}
#ifdef TEST
		cout << delete_quene.size() << " nodes deleted." << endl;
#endif
	}


	void dfs_init(vector<set<int>>& g,  int k, int p, vector<int>& visit, vector<int>& visit1, int depth)
	{
		for (int v : g[k]) {
			if (v < p || visit[v] == 1) {
				continue;
			}
			visit1[v] = p;
			if (depth == 3) {
				continue;
			}
			visit[v] = 1;
			dfs_init(g, v, p, visit, visit1, depth + 1);
			visit[v] = 0;
		}
	}

	void dfs_2(vector<set<int>>& g, int k, int p, vector<int>& visit, vector<int>& visit1, vector<int>& path)
	{
		for (int v : g[k]) {
			if (v < p) continue;
			if (visit1[v] == -2 && visit[v] == 0) {
				path.push_back(v);
				int path_length = path.size();
				if (path_length > 2) {
					int i = 0;
					for (int &x:path)
						path_by_length[path_length][i++] = idx[x];
					cycles_group_by_length[path_length].emplace_back(path_by_length[path_length]);
					++cycle_count;
				}
				path.pop_back();
			}
			if (visit[v] == 1 || (visit1[v] != p && visit1[v] != -2))
				continue;
			if (path.size() == 6 || v == p)
				continue;
			visit[v] = 1;
			path.push_back(v);
			dfs_2(g, v, p, visit, visit1, path);
			path.pop_back();
			visit[v] = 0;
		}
	}


	void cycle_finder_2()
	{
		visit = vector<int>(node_count, 0);
		visit1 = vector<int>(node_count, -1);

		path_by_length.resize(8);
		for (int i = 0; i < 8; ++i) {
			path_by_length[i].resize(i);
		}

		vector<int> path;
		for (int u = 0; u < node_count; ++u) {
			path.push_back(u);
			dfs_init(graph, u, u, visit, visit1, 1);
			dfs_init(graph_pre, u, u, visit, visit1, 1);
			for (int j : graph_pre[u]) {
				visit1[j] = -2;
			}
			dfs_2(graph, u, u, visit, visit1, path);
			path.pop_back();
			for (int j : graph_pre[u]) {
				visit1[j] = u;
			}
		}
#ifdef TEST
		cout << cycle_count << " cyles fonund!" << endl;
#endif // TEST

	}

	void save_answer(string &output_file_path)
	{
		ofstream fout(output_file_path.c_str(), ios::out);
		/*if (!fout.is_open()) {
			cout << "打开result.txt文件失败" << endl;
		}*/
		map<unsigned int, vector<vector<unsigned int>>> ::iterator it = cycles_group_by_length.begin();
		fout << cycle_count << "\n";
		while (it != cycles_group_by_length.end())
		{
			
			for (vector<unsigned int>& path : it->second)
			{
				fout << path[0];
				int sz = path.size();
				for (int i=1; i< sz; ++i)
				{
					fout << "," << path[i];
				}
				fout << "\n";
			}
			++it;
		}
		fout.close();
	}

	void save_answer_2(string& output_file_path)
	{
		/*FILE* file;
		fwrite()
		fopen_s(&file, output_file_path.c_str(), "w");
		map<unsigned int, vector<vector<unsigned int>>> ::iterator it = cycles_group_by_length.begin();
		while (it != cycles_group_by_length.end())
		{
			for (vector<unsigned int> path : it->second)
			{
				cochar delimiter[1] = ",";
				for (unsigned int node : path)
				{
					fwrite(delimiter);
					<< node;
					delimiter = ",";
				}
				fout << "\n";
			}
			++it;
		}
		fout.close();*/
	}

};


int main()
{	
	string input_file_path = "/data/test_data.txt";
	string output_file_path = "/projects/student/result.txt";
	//auto t = clock();
	CycleFinder c;
	//c.load_data(input_file_path);
	c.parseInput(input_file_path);
	//c.parse_input_by_mmap();
	c.init_graph();
	if (c.avg_degree < 4) {
		c.topo_sort();
	}
	//c.get_strong_connected_coms();
	//c.init_path_quene_by_scc();
	/*c.init_path_queue();
	c.cycle_finder();*/
	c.cycle_finder_2();
	c.save_answer(output_file_path);
	//cout << "run time: " << (double)(clock() - t)/ CLOCKS_PER_SEC << " s"<< endl;
	return 0;
}