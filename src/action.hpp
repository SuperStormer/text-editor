#ifndef ACTION_H
#define ACTION_H
#include <iostream>
#include <memory>
#include <string>
#include <vector>
class Action {
   public:
	Action(size_t line, size_t col, std::vector<std::string> lines);
	virtual ~Action() = default;
	virtual std::pair<size_t, size_t> operator()(std::vector<std::string>& lines) = 0;
	virtual std::shared_ptr<Action> reverse() = 0;
	// merges this into action if adjacent
	static std::shared_ptr<Action> merge_if_adj(const std::shared_ptr<Action>& action1, const std::shared_ptr<Action>& action2,
												const std::vector<std::string>& lines);

	const std::pair<size_t, size_t> get_end() const;
	// merges this into action
	void merge(std::shared_ptr<Action> action);
	const size_t line;
	const size_t col;
	std::vector<std::string> lines;
};
class Add : public Action {
	using Action::Action;

   public:
	std::pair<size_t, size_t> operator()(std::vector<std::string>& lines) override;
	std::shared_ptr<Action> reverse() override;
	// std::shared_ptr<Action> merge_if_adj(const std::shared_ptr<Action>& action, const std::vector<std::string>& lines)
	// override;
};
class Remove : public Action {
	using Action::Action;

   public:
	std::pair<size_t, size_t> operator()(std::vector<std::string>& lines) override;
	std::shared_ptr<Action> reverse() override;
	// std::shared_ptr<Action> merge_if_adj(const std::shared_ptr<Action>& action, const std::vector<std::string>& lines)
	// override;
};
#endif