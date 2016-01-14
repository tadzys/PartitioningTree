#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <memory>

using namespace std;

class PartitioningTree
{
public:
	/**
	Structure to represent the partitioning tree
	root...
		a...
			amazon...
				amazon
				amazon-adsystem
				amazonwebapps
		aol
		apple
	leaf with 3 dots (...) is a partition, so is_leaf == false
	leaf without dots is just a leaf, is_leaf_ == true
	*/
	typedef struct NodeType
	{
		NodeType()
			:parent_(),
			is_leaf_(true)
		{}

		string partition_;
		bool is_leaf_;
		vector<shared_ptr<NodeType>> nodes_;
		weak_ptr<NodeType> parent_; //points to a parent leaf, so we could climb the tree up

		//prints a tree
		static void print(NodeType* p, int level = 0)
		{
			auto print_with_identation = [](int count, const string& data) -> ostream& { 
				for (int i = 0; i < count; ++i) { cout << "\t"; }
				cout << data;
				return cout;
			};

			(p->is_leaf_) ? (print_with_identation(level, p->partition_) << endl)
				: (print_with_identation(level, p->partition_ + "...") << endl);

			for (auto& i : p->nodes_)
				print(i.get(), level + 1);
		}

	} NodeType;

	PartitioningTree(const vector<string>& input, string::size_type tz)
		: TargetSize(tz)
	{
		 root_ = make_shared<NodeType>();
		 root_->partition_ = "Root";
		 root_->is_leaf_ = false;

		construct_tree(input);
	}
	friend ostream& operator<< (ostream &out, const PartitioningTree&);

private:
	shared_ptr<NodeType> root_;
	const string::size_type TargetSize;

protected:

	void construct_tree(const vector<string>& input)
	{
		//helper function to create Nodes
		auto create_node = [](const string& name, shared_ptr<NodeType>& parent) {
			shared_ptr<NodeType> node = make_shared<NodeType>();
			node->partition_ = name;
			node->parent_ = parent;
			return node;
		};

		string previous_word = *input.begin();
		auto current_partition = create_node(previous_word, root_);
		root_->nodes_.push_back(current_partition);

		//another helper to construct partition
		auto create_partition = [&](const string& partition) {
			auto p = make_shared<NodeType>();
			p->partition_ = partition;
			p->is_leaf_ = false;
			p->nodes_.push_back(current_partition);
			p->parent_ = current_partition->parent_;
			current_partition = p;
			root_->nodes_[root_->nodes_.size() - 1] = current_partition;
		};

		auto find_slot = [this](shared_ptr<NodeType>& parent) mutable {
			while (parent->nodes_.size() >= TargetSize)
			{
				if (parent->parent_.lock() == root_) //we stop before the root
					break;
				parent = parent->parent_.lock();
			}
		};

		for (auto i = ++input.begin(); i != input.end(); ++i)
		{
			//find the location where the mismatch between the strings begin
			string::size_type n = mismatch(previous_word.begin(), previous_word.end(), i->begin(), i->end()).first - previous_word.begin();
			//there is a common prefix
			if (n > 0)
			{
				//if parent is not a partition but just a leaf, need to create one
				if (current_partition->is_leaf_)
				{
					create_partition(previous_word.substr(0, n));
				}

				//current partition still has space
				if (current_partition->nodes_.size() < TargetSize)
				{
					if (n >= current_partition->partition_.length())
					{
						current_partition->nodes_.push_back(create_node(*i, current_partition));
					}
					else if (n < current_partition->partition_.length())
					{
						//we need to change partition to a shorter one, as we need to be as close to target size
						find_slot(current_partition);

						current_partition->partition_ = current_partition->partition_.substr(0, n);
						current_partition->nodes_.push_back(create_node(*i, current_partition));
					}
				}
				//current partition is full
				else
				{
					//creating a new sub partition	
					if (n > current_partition->partition_.length())
					{
						auto index = current_partition->nodes_.size() - 1;
						//change current node to partition
						auto temp = current_partition->nodes_[index]->partition_;
						current_partition = current_partition->nodes_[index];
						current_partition->is_leaf_ = false;
						//create needed nodes
						current_partition->nodes_.push_back(create_node(temp, current_partition));
						current_partition->nodes_.push_back(create_node(*i, current_partition));
					}
					//going up the tree to find a space
					else
					{
						find_slot(current_partition);
						//need to change partition name to a shorter one
						if (n < current_partition->partition_.length())
						{
							create_partition((*i).substr(0, n));
							current_partition->nodes_.push_back(create_node((*i), current_partition));
						}
						else
						{
							current_partition->nodes_.push_back(create_node((*i), current_partition));
						}
					}
				}
			}
			//No common prefix, therefore it is different partition
			else
			{
				auto p = create_node(*i, root_);
				root_->nodes_.push_back(p);
				current_partition = p;
			}
			previous_word = (*i);
		}
	}
};

ostream& operator<< (ostream &out, const PartitioningTree &tree)
{
	PartitioningTree::NodeType::print(tree.root_.get());
	return cout;
}


int main(int argc, char* argv)
{
	vector<string> input = { "amaz", "amazi", "amazon", "amazon - adsystem", "amazonwebapps", "aol", "apple", "google", "google - analytics", "googledrive", "googlesyndication", "googlevideo", "nexthink", "nexthink - website", "netflix" };
	//vector<string> input = { "amazon", "amazon-adsystem", "amazonwebapps", "aol", "apple", "google", "google - analytics", "googledrive", "googlesyndication", "googlevideo", "nexthink", "nexthink - website", "netflix" };
	//vector<string> input = { "aam", "aamb", "aamc", "aamcc", "aamcd", "aamce", "aau", "ap", "ac" };
	//vector<string> input = { "aam", "aamb"};
	sort(input.begin(), input.end());

	PartitioningTree a(input, 3);

	cout << a;
}