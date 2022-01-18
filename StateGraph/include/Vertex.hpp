#pragma once

#include <string>
#include <utility>

namespace bringauto {

class Vertex {
public:
	explicit Vertex(std::string newName) : name{std::move(newName)} {};
	const std::string name;
};

}