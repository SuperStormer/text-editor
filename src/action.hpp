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
	size_t line;
	size_t col;
	std::vector<std::string> lines;
};
class Add : public Action {
	using Action::Action;

   public:
	std::pair<size_t, size_t> operator()(std::vector<std::string>& lines) override;
	std::shared_ptr<Action> reverse() override;
};
class Remove : public Action {
	using Action::Action;

   public:
	std::pair<size_t, size_t> operator()(std::vector<std::string>& lines) override;
	std::shared_ptr<Action> reverse() override;
};
#endif