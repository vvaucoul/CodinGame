//**************************************************//
//													//
//					  INCLUDES						//
//													//
//**************************************************//

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <list>
#include <algorithm>
#include <map>
#include <chrono>
#include <ctime>
#include <climits>

using namespace std;

std::vector<std::vector<int>> _allNeights {
{ 1, 2, 3, 4, 5, 6 },
{ 7, 8, 2, 0, 6, 18 },
{ 8, 9, 10, 3, 0, 1 },
{ 2, 10, 11, 12, 4, 0 },
{ 0, 3, 12, 13, 14, 5 },
{ 6, 0, 4, 14, 15, 16 },
{ 18, 1, 0, 5, 16, 17 },
{ 19, 20, 8, 1, 18, 36 },
{ 20, 21, 9, 2, 1, 7 },
{ 21, 22, 23, 10, 2, 8 },
{ 9, 23, 24, 11, 3, 2 },
{ 10, 24, 25, 26, 12, 3 },
{ 3, 11, 26, 27, 13, 4 },
{ 4, 12, 27, 28, 29, 14 },
{ 5, 4, 13, 29, 30, 15 },
{ 16, 5, 14, 30, 31, 32 },
{ 17, 6, 5, 15, 32, 33 },
{ 35, 18, 6, 16, 33, 34 },
{ 36, 7, 1, 6, 17, 35 },
{ -1, -1, 20, 7, 36, -1 },
{ -1, -1, 21, 8, 7, 19 },
{ -1, -1, 22, 9, 8, 20 },
{ -1, -1, -1, 23, 9, 21 },
{ 22, -1, -1, 24, 10, 9 },
{ 23, -1, -1, 25, 11, 10 },
{ 24, -1, -1, -1, 26, 11 },
{ 11, 25, -1, -1, 27, 12 },
{ 12, 26, -1, -1, 28, 13 },
{ 13, 27, -1, -1, -1, 29 },
{ 14, 13, 28, -1, -1, 30 },
{ 15, 14, 29, -1, -1, 31 },
{ 32, 15, 30, -1, -1, -1 },
{ 33, 16, 15, 31, -1, -1 },
{ 34, 17, 16, 32, -1, -1 },
{ -1, 35, 17, 33, -1, -1 },
{ -1, 36, 18, 17, 34, -1 },
{ -1, 19, 7, 18, 35, -1 }};

//**************************************************//
//													//
//					  DEFINES						//
//													//
//**************************************************//

# define NUMBER_OF_CELLS	37
# define MAX_NEIGH			6

//**************************************************//
//													//
//					  STRUCTURES					//
//													//
//**************************************************//

typedef struct	s_Action
{
	int 		actionType = -1;
	int 		treeLocation = -1;
	int 		seedLocation = -1;
	int 		growLocation = -1;
	int 		completeLocation = -1;
}				t_Action;

typedef struct	s_gameInfo
{
	int		day;
	int		nutrients;
	int		sun;
	int		score;
	int		oppSun;
	int		oppScore;
	bool	oppIsWaiting;
	int		numberOfTrees;
}				t_gameInfo;

typedef struct s_plateau
{
   int		index;
   int		richness;
   int		neigh0;
   int		neigh1;
   int		neigh2;
   int		neigh3;
   int		neigh4;
   int		neigh5;
}				t_plateau;

//**************************************************//
//													//
//					  PROTOTYPES					//
//													//
//**************************************************//

int SortByTreeLocation(const t_Action &action, const t_Action &action2);

//**************************************************//
//													//
//					  CLASSES						//
//													//
//**************************************************//

class Tree {
private:
	  int _cellIndex = 0;
	  int _size = 0;
	  bool _isMine = 0;
	  bool _isDormant = 0;

public:
	Tree (int cellIndex, int size, bool isMine, bool isDormant) :
	_cellIndex(cellIndex), _size(size), _isMine(isMine), _isDormant(isDormant) {};

	virtual ~Tree () {};

	int getCellIndex() { return (_cellIndex); }
	int getSize() { return (_size); }
	bool isMine() { return (_isMine); }
	bool isDormant() { return (_isDormant); }

};

class Game {
protected:
	std::vector<string>		_possibleActions;
	std::vector<t_plateau>	_plateau;
	t_gameInfo				_gameInfo;
	std::vector<Tree>		_trees;

public:
	Game () {};
	virtual ~Game () {};

	void updatePlateau(std::vector<t_plateau> plateau) {
		_plateau.clear();
		_plateau = plateau;
	}
	void updateGameInfo(t_gameInfo info) {
		_gameInfo = info;
	}
	void updateTrees(std::vector<Tree> trees) {
		_trees.clear();
		_trees = trees;
	}
	void updatePossibleActions(std::vector<string> actions) {
		_possibleActions.clear();
		_possibleActions = actions;
	}

	std::vector<std::string>	getAllPossibleAction(std::string type);
	std::vector<t_Action>		parseAllPossibleActions(std::string type);

	std::vector<Tree>			getTrees() {
		return (_trees);
	}
	std::vector<t_plateau>		getPlateau() {
		return (_plateau);
	}
	t_gameInfo					getGameInfo() {
		return (_gameInfo);
	}

	int numberOfTrees(std::vector<Tree> trees, int size) {
		int nb = 0;

		if (size > 3 || size < 0)
			return (0);
		for (std::vector<Tree>::iterator it = trees.begin(); it != trees.end(); ++it){
			if (!it->isMine())
				continue;
			else if (it->getSize() == size)
				++nb;
		}
		return (nb);
	}
	// Action : 0 -> wait | 1 -> seed | 2 -> grow | 3 -> complete
	bool	hasEnoughSun(int action, int currentSun, std::vector<Tree> trees, int treeSize) {

		switch (action) {
			case 0:
				return (0);
			case 1:
				return (currentSun >= numberOfTrees(trees, 0));
			case 2:
			{
				switch (treeSize) {
					case 0:
						return (currentSun >= numberOfTrees(trees, 0));
					case 1:
						return (currentSun >= (1 + numberOfTrees(trees, 1)));
					case 2:
						return (currentSun >= (3 + numberOfTrees(trees, 1)));
					case 3:
						return (currentSun >= (7 + numberOfTrees(trees, 1)));
				}
			}
			case 3:
				return (currentSun >= 4);
		}
		return (false);
	}

	int	getNeighPosition(t_plateau plateau, int index) {
		switch (index) {
			case 0:
				return (plateau.neigh0);
			case 1:
				return (plateau.neigh1);
			case 2:
				return (plateau.neigh2);
			case 3:
				return (plateau.neigh3);
			case 4:
				return (plateau.neigh4);
			case 5:
				return (plateau.neigh5);
		}
		return (-1);
	}

	bool isInVector(std::vector<int> ref, int value) {
		for (std::vector<int>::iterator it = ref.begin(); it != ref.end(); ++it)
			if (*it == value)
				return (true);
		return (false);
	}

	// a fix, ne fonctionne pas
	// return neighIndex & distanceTo tree, 1, 2, 3
	std::map<std::vector<int>, int>	getAllNeigh(int treeIndex, int treeSize, std::vector<t_plateau> plateau) {

		t_plateau						cellRef = plateau[treeIndex];

		std::map<std::vector<int>, int>	neights;

		// first neights

		//  faire 3 boucles, stoquer a chaque fois tous les voisins et enlever dans voisins 2, les voisins 1 et voisins 3, voisins 2 et 1


		std::vector<int>	neightsD1;
		std::vector<int> 	neightsD2;
		std::vector<int> 	neightsD3;

		for (int i = 0; i < MAX_NEIGH; ++i) {
			int cn = getNeighPosition(cellRef, i);

			if (cn == treeIndex
				|| cn == -1)
				continue;

			neightsD1.push_back(cn);
			//fprintf(stderr, "- Add Neigh [%d] [%d]\n", cn, 1);
		}


		for (std::vector<int>::iterator it = neightsD1.begin(); it != neightsD1.end(); ++it) {
			cellRef = plateau[*it];
			for (int i = 0; i < MAX_NEIGH; ++i) {
				int cn = getNeighPosition(cellRef, i);

				if (cn == treeIndex
				|| cn == -1
				|| isInVector(neightsD1, cn)
				|| isInVector(neightsD2, cn))
					continue;

				neightsD2.push_back(cn);
				//fprintf(stderr, "- Add Neigh [%d] [%d]\n", cn, 2);

			}
		}

		for (std::vector<int>::iterator it = neightsD2.begin(); it != neightsD2.end(); ++it) {
			cellRef = plateau[*it];
			for (int i = 0; i < MAX_NEIGH; ++i) {
				int cn = getNeighPosition(cellRef, i);

				if (cn == treeIndex
				|| cn == -1
				|| isInVector(neightsD1, cn)
				|| isInVector(neightsD2, cn)
				|| isInVector(neightsD3, cn))
					continue;

				neightsD3.push_back(cn);
				//fprintf(stderr, "- Add Neigh [%d] [%d]\n", cn, 3);

			}
		}



		for (std::vector<int>::iterator it = neightsD3.begin(); it != neightsD3.end(); ++it) {
			fprintf(stderr, "Neigh [Distance 3] : %d\n", *it);
		}

		for (std::vector<int>::iterator it = neightsD2.begin(); it != neightsD2.end(); ++it) {
			fprintf(stderr, "Neigh [Distance 2] : %d\n", *it);
		}

		for (std::vector<int>::iterator it = neightsD1.begin(); it != neightsD1.end(); ++it) {
			fprintf(stderr, "Neigh [Distance 1] : %d\n", *it);
		}

		cerr << "\n#####\n\n";

		neights.emplace(neightsD1, 1);
		neights.emplace(neightsD2, 2);
		neights.emplace(neightsD3, 3);

		return (neights);
	};

};

//**************************************************//
//													//
//					  BFS							//
//													//
//**************************************************//

namespace BFS {

	typedef struct		s_bfs
	{
		bool is_valid;
		bool visited;
		int index;
		int *neigh;
	}					t_bfs;

	class BFS {
	private:
		std::vector<t_bfs>		_bfs;
		std::vector<int>		_adj[NUMBER_OF_CELLS];
		std::vector<int>		_resultPath;

	// BFS ALGO //
	bool getPath(int src, int dest, int size, int pred[], int dist[]) {
		std::list<int> queue;
		bool visited[size];

		// initialisation vertices
		for (int i = 0; i < size; ++i)
		{
			visited[i] = false;
			dist[i] = INT_MAX;
			pred[i] = -1;
		}

		visited[src] = true;
		dist[src] = 0;
		queue.push_back(src);

		while (!queue.empty())
		{
			int u = queue.front();
			queue.pop_front();

			for (int i = 0; i < _adj[u].size(); ++i)
			{
				if (visited[_adj[u][i]] == false){
					visited[_adj[u][i]] = true;
					dist[_adj[u][i]] = dist[u] + 1;
					pred[_adj[u][i]] = u;
					queue.push_back(_adj[u][i]);

					if (_adj[u][i] == dest)
					return (true);
				}
			}
		}
		return (false);
	}
	void addEdge(int src, int dest) {
		_adj[src].push_back(dest);
		_adj[dest].push_back(src);
	}
	int	getNeighPosition(t_plateau plateau, int index) {
		switch (index) {
			case 0:
				return (plateau.neigh0);
			case 1:
				return (plateau.neigh1);
			case 2:
				return (plateau.neigh2);
			case 3:
				return (plateau.neigh3);
			case 4:
				return (plateau.neigh4);
			case 5:
				return (plateau.neigh5);
		}
		return (-1);
	}

	int isNotVisited(int x, std::vector<int> &path){
		int size = path.size();
		for (int i = 0; i < size; ++i) {
			if (path[i] == x)
				return (0);
		}
		return (1);
	}

	public:
		BFS () {};
		virtual ~BFS () {};

	public :

		// get returnValues //
		std::vector<int> getResultPath() { return (_resultPath); }
		size_t	getResultLength() {
			int nb = 0;

			if (_resultPath.empty())
				return (-1);
			for (std::vector<int>::iterator it = _resultPath.begin(); it != _resultPath.end(); ++ it)
				++nb;
			return (nb);
		}

		bool getBFSPath(int src, int dest, int size) {

			std::queue<std::vector<int> >	q;
			std::vector<int>				path;

			path.push_back(src);
			q.push(path);

			while (!q.empty()) {
				path = q.front();
				q.pop();
				int last = path[path.size() - 1];

				if (last == dest) {
					//printPath(path);
					_resultPath = path;
					return (true);
				}
				for (int i = 0; i < _adj[last].size(); ++i) {
					if (isNotVisited(_adj[last][i], path)) {
						std::vector<int> newPath(path);
						newPath.push_back(_adj[last][i]);
						q.push(newPath);
					}
				}
			}
			_resultPath = path;
			return (false);

			/*
			int pred[NUMBER_OF_CELLS], dist[NUMBER_OF_CELLS];

			if (getPath(src, dest, size, pred, dist) == false)
				return (false);
			std::vector<int> path;
			int crawl = dest;
			path.push_back(crawl);
			while (pred[crawl] != -1) {
				path.push_back(pred[crawl]);
				crawl = pred[crawl];
			}
			_resultPath = path;
			std::reverse(_resultPath.begin(), _resultPath.end());
			_resultPath.erase(_resultPath.begin());
			return (true);
			*/
		}

		void printPath(std::vector<int> &path) {
			int size = path.size();
			for (int i = 0; i < size; ++i) {
				cerr << path[i] << " ";
			}
			cerr << endl;
		}

		// INIT BFS //
		void initBFS(std::vector<t_plateau> plateau) {
			for (int i = 0; i < NUMBER_OF_CELLS; ++i) {
				for (int j = 0; j < MAX_NEIGH; ++j) {
					if (getNeighPosition(plateau[i], j) != -1)
						addEdge(i, getNeighPosition(plateau[i], j));
				}
			}
		}

	};
};

//**************************************************//
//													//
//					  MCTS							//
//													//
//**************************************************//

/*
** Principe :
** Creer des couches de map, dans chacune, creer une action possible jusqu a la fin de la partie
** recuperer a  l instant T, la meilleur,
** update jusqu a la fin de partie
*/

namespace MCTS {

	typedef struct s_NodeMap
	{
		int						index = -1;
		std::vector<Tree>		tree;
		std::vector<t_plateau>	plateau;

		int						nodeIndex = 0;
		int						sunScore = 0;
		int						nutrients = 0;
	}				t_NodeMap;

	typedef struct s_Node
	{
		struct s_Node		*left;
		struct s_Node		*right;
		t_NodeMap			*currentNodeMap;
	}				t_Node;

	class mcts : public Game {



	private:

		t_Node	*_rootNode;

	public:
		mcts () {};
		virtual ~mcts () {
			delete (_rootNode);
		};

		// Nodes Values
	public :

		t_Node *CreateNode(t_NodeMap *data) {
			t_Node *newNode = new (t_Node);

			if (!newNode) {
				cerr << "MEMORY ERROR" << endl;
				return (NULL);
			}
			newNode->left = NULL;
			newNode->right = NULL;
			newNode->currentNodeMap = data;
			return (newNode);
		}
		t_Node	*insertNode(t_Node *root, t_NodeMap *data) {
			if (root == NULL) {
				root = CreateNode(data);
				return (root);
			}

			std::queue<t_Node *> q;
			q.push(root);

			while (!q.empty())
			{
				t_Node *tmp = q.front();
				q.pop();

				if (tmp->left != NULL)
				q.push(tmp->left);
				else {
					tmp->left = CreateNode(data);
					return (root);
				}

				if (tmp->right != NULL)
				q.push(tmp->right);
				else {
					tmp->right = CreateNode(data);
					return (root);
				}
			}
			return (NULL);
		}
		void 	printTree(t_Node *node) {
			if (node == NULL)
				return ;
			printTree(node->left);
			cout << "\t- " << node->currentNodeMap->index << endl;
			printTree(node->right);
		}

	public :

		// obtenir toutes les actions possible avec le plateau actuel

		std::vector<t_Action>	getAllSeedActions(std::vector<Tree> allTrees, t_NodeMap *refNode) {
			std::vector<t_Action> allActions;

			BFS::BFS	bfs;
			bfs.initBFS(refNode->plateau);

			for (std::vector<Tree>::iterator it = allTrees.begin(); it != allTrees.end(); ++it) {
				t_Action currentAction;

				if (it->isDormant()
				||	!it->isMine()
				||	!hasEnoughSun(1, refNode->sunScore, allTrees, it->getSize()))
					continue;

				cerr << "TEST ALL NEIGH" << endl;
				cerr << "REF : " << it->getCellIndex() << endl;
				std::map<std::vector<int>, int>	allNeigh = getAllNeigh(it->getCellIndex(), it->getSize(), refNode->plateau);

				for (std::map<std::vector<int>, int>::iterator it = allNeigh.begin(); it != allNeigh.end(); ++it) {

					std::vector<int> nbd = it->first;
					cerr << "Neights distance : " << it->second;
					for (std::vector<int>::iterator it2 = nbd.begin(); it2 != nbd.end(); ++ it2) {
						cerr << *it2 << " " << endl;
					}
					cerr << endl << endl;
				}
				cerr << endl;

				//for (int i = 0; i < NUMBER_OF_CELLS; ++i) {
				for (int i = 0; i < allNeigh.size(); ++i) {


					//if (it->getCellIndex() == i)
					//	continue ;

					//bool validPath = bfs.getBFSPath(it->getCellIndex(), i, it->getSize());

					//cerr << "BFS PATH LEN = " << bfs.getResultLength() << " TI :  " << it->getCellIndex() << " CI : " << i << endl;
					//cerr << "Result Path = ";

					//std::vector<int> result = bfs.getResultPath();

					//for (std::vector<int>::iterator it = result.begin(); it != result.end(); ++it)
					//	cerr << *it << " ";
					//cerr << endl;

					//if (bfs.getResultLength() - 1 > it->getSize()
					//|| !validPath
					//|| bfs.getResultLength() - 1 <= 0)
					//	continue ;

					currentAction.actionType = 1;
					currentAction.treeLocation = it->getCellIndex();
					currentAction.seedLocation = i;

					allActions.push_back(currentAction);
				}
			}
			return (allActions);
		}
		std::vector<t_Action>	getAllGrowActions(std::vector<Tree> allTrees, t_NodeMap *refNode) {
			std::vector<t_Action> allActions;

			for (std::vector<Tree>::iterator it = allTrees.begin(); it != allTrees.end(); ++it) {

				t_Action currentAction;

				if (it->isDormant()
				||	!it->isMine()
				||	it->getSize() >= 3
				|| !hasEnoughSun(2, refNode->sunScore, allTrees, it->getSize()))
					continue;

				currentAction.actionType = 2;
				currentAction.treeLocation = it->getCellIndex();
				currentAction.growLocation = it->getCellIndex();

				allActions.push_back(currentAction);
			}

			return (allActions);
		}
		std::vector<t_Action>	getAllCompleteActions(std::vector<Tree> allTrees, t_NodeMap *refNode) {

			std::vector<t_Action> allActions;

			for (std::vector<Tree>::iterator it = allTrees.begin(); it != allTrees.end(); ++it) {

				t_Action	currentAction;

				if (it->isDormant()
				|| !it->isMine()
				|| it->getSize() < 3
				|| !hasEnoughSun(3, refNode->sunScore, allTrees, it->getSize()))
					continue;

				currentAction.actionType = 3;
				currentAction.treeLocation = it->getCellIndex();
				currentAction.completeLocation = it->getCellIndex();

				allActions.push_back(currentAction);
			}
			return (allActions);
		}

		std::vector<t_Action> getAllPossibleAction(t_NodeMap *refNode) {

			std::vector<t_Action>	allActions;
			std::vector<t_Action> currentAction;

			// Seeds

			cerr << "1" << endl;
			currentAction = getAllSeedActions(refNode->tree, refNode);
			cerr << "2" << endl;

			for (std::vector<t_Action>::iterator it = currentAction.begin(); it != currentAction.end(); ++it)
				fprintf(stderr, "SEED ACTION | Tree [%d] to case [%d]\n", it->treeLocation, it->seedLocation);

			allActions.insert(allActions.end(), currentAction.begin(), currentAction.end());

			// Grow
			currentAction.clear();
			currentAction = getAllGrowActions(refNode->tree, refNode);
			allActions.insert(allActions.end(), currentAction.begin(), currentAction.end());

			// Complete
			currentAction.clear();
			currentAction = getAllCompleteActions(refNode->tree, refNode);
			allActions.insert(allActions.end(), currentAction.begin(), currentAction.end());

			std::sort(allActions.begin(), allActions.end(), SortByTreeLocation);
			return (allActions);
		}

		t_NodeMap *createNewNodeMap(t_NodeMap *refNode) {

			t_NodeMap *newNode = new (t_NodeMap);

			std::vector<t_Action> allActions = getAllPossibleAction(refNode);

			for (size_t i = 0; i < NUMBER_OF_CELLS; i++) {


			}

			return (newNode);
		}

		int						generatePossibilities(t_NodeMap *refNode) {

			t_NodeMap *currentNodeMap = refNode;

			while (true) {
				currentNodeMap = createNewNodeMap(refNode);
				insertNode(_rootNode, currentNodeMap);
				refNode = currentNodeMap;
			}

		}

	};
};

//**************************************************//
//													//
//					  INIT							//
//													//
//**************************************************//

std::vector<t_plateau>	initPlateau() {
	std::vector<t_plateau> plateau;
	int numberOfCells;

    cin >> numberOfCells; cin.ignore();
    for (int i = 0; i < numberOfCells; i++) {
        int index;
        int richness;
        int neigh0;
        int neigh1;
        int neigh2;
        int neigh3;
        int neigh4;
        int neigh5;
        cin >> index >> richness >> neigh0 >> neigh1 >> neigh2 >> neigh3 >> neigh4 >> neigh5; cin.ignore();
		plateau.push_back(t_plateau{index, richness, neigh0, neigh1, neigh2, neigh3, neigh4, neigh5});
    }
	return (plateau);
}
t_gameInfo				initGameInfo() {
	t_gameInfo info;

	cin >> info.day; cin.ignore();
	cin >> info.nutrients; cin.ignore();
	cin >> info.sun >> info.score; cin.ignore();
	cin >> info.oppSun >> info.oppScore >> info.oppIsWaiting; cin.ignore();
	return (info);
}
std::vector<Tree>		initTrees() {
	std::vector<Tree> trees;
	int numberOfTrees;

	cin >> numberOfTrees; cin.ignore();
	for (int i = 0; i < numberOfTrees; i++) {
		int cellIndex;
		int size;
		bool isMine;
		bool isDormant;
		cin >> cellIndex >> size >> isMine >> isDormant; cin.ignore();

		Tree currentTree(cellIndex, size, isMine, isDormant);
		trees.push_back(currentTree);
	}
	return (trees);
}
std::vector<string>		initPossibleAction() {
	std::vector<string> possibleActions;

	int numberOfPossibleActions;
	cin >> numberOfPossibleActions; cin.ignore();
	for (int i = 0; i < numberOfPossibleActions; i++) {
		string possibleAction;
		getline(cin, possibleAction);

		possibleActions.push_back(possibleAction);
	}
	return (possibleActions);
}

//**************************************************//
//													//
//					  PARSING						//
//													//
//**************************************************//

std::vector<std::string>	Game::getAllPossibleAction(std::string type) {
	std::vector<std::string> result;

	for (std::vector<std::string>::iterator it = _possibleActions.begin(); it != _possibleActions.end(); ++it)
	{
		if ((it->find(type)) != std::string::npos)
			result.push_back(*it);
	}
	return (result);
}
std::vector<t_Action> Game::parseAllPossibleActions(std::string type) {
	std::vector<t_Action> Actions;
	std::vector<std::string> allPossibleActions = getAllPossibleAction(type);

	for (std::vector<std::string>::iterator it = allPossibleActions.begin(); it != allPossibleActions.end(); ++it)
	{
		std::string currentAction = *it;
		t_Action	Action;

		if (currentAction.find("SEED") != std::string::npos)
		{
			Action.actionType = 1;
			currentAction.erase(currentAction.begin(), currentAction.begin() + 5);
			std::string treeLocation = "";
			std::string seedLocation = "";
			treeLocation = currentAction.substr(0, currentAction.find(" "));
			seedLocation = currentAction.substr(currentAction.find(" ") + 1, 99);

			Action.treeLocation = std::stoi(treeLocation);
			Action.seedLocation = std::stoi(seedLocation);
			Actions.push_back(Action);
		}

		if (currentAction.find("GROW") != std::string::npos)
		{
			Action.actionType = 2;
			currentAction.erase(currentAction.begin(), currentAction.begin() + 5);
			std::string growLocation = "";
			growLocation = currentAction.substr(0, 99);
			Action.growLocation = std::stoi(growLocation);
			Actions.push_back(Action);
		}

		if (currentAction.find("COMPLETE") != std::string::npos)
		{
			Action.actionType = 3;
			currentAction.erase(currentAction.begin(), currentAction.begin() + 9);
			std::string completeLocation = "";
			completeLocation = currentAction.substr(0, 99);
			Action.completeLocation = std::stoi(completeLocation);
			Actions.push_back(Action);
		}
	}
	std::sort(Actions.begin(), Actions.end(), SortByTreeLocation);
	return (Actions);
}
int SortByTreeLocation(const t_Action &action, const t_Action &action2) {
	return (action.treeLocation < action2.treeLocation);
}

//**************************************************//
//													//
//					  GAME LOOP						//
//													//
//**************************************************//

int main()
{
	Game					game;
	std::vector<t_plateau>	plateau = initPlateau();;

    // game loop
    while (1) {


		// Start Chrono
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		// Update all Values
		game.updatePlateau(plateau);
		game.updateGameInfo(initGameInfo());
		game.updateTrees(initTrees());
		game.updatePossibleActions(initPossibleAction());

		// OLD - UpdatePossibleActions
		/*
		std::vector<t_Action>	apaSeed = game.parseAllPossibleActions("SEED");
		std::vector<t_Action>	apaGrow = game.parseAllPossibleActions("GROW");
		std::vector<t_Action>	apaComplete = game.parseAllPossibleActions("COMPLETE");
		for (size_t i = 0; i < apaSeed.size(); i++) {
			fprintf(stderr, "\t[%ld] [%d] | [%d] -> [%d]\n", i, apaSeed[i].actionType, apaSeed[i].treeLocation ,apaSeed[i].seedLocation);
		}
		for (size_t i = 0; i < apaGrow.size(); i++) {
			fprintf(stderr, "\t[%ld] [%d] | [%d]\n", i, apaGrow[i].actionType, apaGrow[i].growLocation);
		}
		for (size_t i = 0; i < apaComplete.size(); i++) {
			fprintf(stderr, "\t[%ld] [%d] | [%d]\n", i, apaComplete[i].actionType, apaComplete[i].completeLocation);
		}
		*/

		cerr << "\n### MCTS ###\n" << endl;

		MCTS::mcts mcts;

		cerr << "DEBUG [GetAllPossibleActions]" << endl;

		MCTS::t_NodeMap *nodeMap = new (MCTS::t_NodeMap);

		nodeMap->index = 0;
		nodeMap->tree = game.getTrees();
		nodeMap->plateau = game.getPlateau();
		nodeMap->nodeIndex = 0;
		nodeMap->sunScore = game.getGameInfo().sun;
		nodeMap->nutrients = game.getGameInfo().nutrients;

		//MCTS::t_Node *rootNode = mcts.CreateNode(nodeMap);

		std::vector<t_Action> actions = mcts.getAllPossibleAction(nodeMap);

		for (std::vector<t_Action>::iterator it = actions.begin(); it != actions.end(); ++it)
		{
			fprintf(stderr, "\tTYPE : [%d] | TREE LOCATION : [%d] | GROW LOCATION : [%d] | SEED LOCATION : [%d]\n", it->actionType, it->treeLocation, it->growLocation, it->seedLocation);
		}

		MCTS::t_NodeMap *newMap = mcts.createNewNodeMap(nodeMap);


		cerr << "\n### END MCTS ###" << endl;

		// creer une map pour chaque tableau des actions

		// Chrono
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cerr << "Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

		cout << "GROW 27" << endl;

        cout << "WAIT" << endl;
    }
}

// todo : debug, une fonction loop infinie
