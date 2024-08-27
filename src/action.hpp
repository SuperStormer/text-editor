#ifndef ACTION_H
#define ACTION_H
#include <memory>
#include <string>
#include <vector>

#include "position.hpp"
class Action {
   public:
	Action(size_t line, size_t col, std::vector<std::string> lines);
	virtual ~Action() = default;
	Action(const Action& action) = default;
	Action(Action&& action) = default;
	virtual Position operator()(std::vector<std::string>& lines) = 0;
	virtual std::shared_ptr<Action> reverse() = 0;
	// merges action2 into action1 if adjacent
	static std::shared_ptr<Action> merge_if_adj(const std::shared_ptr<Action>& action1,
												const std::shared_ptr<Action>& action2,
												const std::vector<std::string>& lines);

	[[nodiscard]] Position get_end() const;
	const size_t line;
	const size_t col;
	std::vector<std::string> lines;
};
class Add : public Action {
	using Action::Action;

   public:
	Position operator()(std::vector<std::string>& lines) override;
	std::shared_ptr<Action> reverse() override;
};
class Remove : public Action {
	using Action::Action;

   public:
	Position operator()(std::vector<std::string>& lines) override;
	std::shared_ptr<Action> reverse() override;
};
#endif