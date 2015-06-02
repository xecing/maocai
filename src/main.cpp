#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <ctype.h>
#include <functional>
#include <vector>
#include <list>

enum TokenKind
{
	TOKEN_BAD,
	TOKEN_NUM,
	TOKEN_ADD,
	TOKEN_SUB,
	TOKEN_MUL,
	TOKEN_DIV,
	TOKEN_BRAKET_OPEN,
	TOKEN_BRAKET_CLOSE
};

enum LexerStatus
{
	STATUS_PARSE_INIT,
	STATUS_PARSE_INT,
	STATUS_PARSE_REAL,
	STATUS_PARSE_DOT
};

enum ErrorCode
{
	ERROR_SUCCESS,
	ERROR_BADTOKEN
};

struct Token
{
	TokenKind 		type;
	std::string	   	type_str;
	double	  		value;
	std::string 	text;
};

struct State
{
	std::function< bool (const char c, Token& t)> onGuard;
	std::function< void (const char c, Token& t)> onEnter;
	std::function< void (const char c, Token& t)> onExit;
	std::function< void (const char c, Token& t)> onError;
	std::string 	 name;
	State(std::string name) 
		: onGuard(nullptr)
		, onEnter(nullptr)
		, onExit(nullptr)
		, onError(nullptr)
		, name(name)
	{
	}
};

struct Lexter
{
	Lexter()
	{
		init_num_state();
		init_add_state();
		init_sub_state();
		init_mul_state();
		init_div_state();
		init_space_state();
		init_braket_state();
		init_bad_state();
	}
	void Parse(std::string line)
	{
		Token t;
		for (size_t i = 0; i < line.length(); ++i)
		{
			for (auto s : states)
			{
				if (s.onGuard && s.onGuard(line[i], t))
				{
					if (s.onEnter)
					{
						s.onEnter(line[i], t);
					}

					if (s.onExit)
					{
						s.onExit(line[i+1], t);
					}
					break;
				}
			}
		}
	}

	void Print()
	{
		for (auto t : tokens)
		{
			if (t.type == TOKEN_NUM)
			{
				std::cout<<"Token\t"<<t.text<<"\tType\t"<<t.type_str<<"\tValue\t"<<t.value<<std::endl;
			}
			else
			{
				std::cerr<<"Token\t"<<t.text<<"\tType\t"<<t.type_str<<std::endl;
			}
		}
	}

	void init_bad_state()
	{
		State BadTokenState("BadToken");
		BadTokenState.onGuard = [](const char c, Token& t) -> bool
		{
			return true;
		};

		BadTokenState.onEnter = [](const char c, Token& t) -> void
		{
			t.text 	 	= c;
			t.type 	 	= TOKEN_BAD;
			t.type_str	= "Bad";
		};

		BadTokenState.onExit  = [this](const char c, Token& t) -> void
		{
			std::cerr<<"fatal error:"<<std::endl;
			std::cerr<<"\tBadToken\t"<<t.text<<std::endl;
			std::cerr<<"==============================="<<std::endl;
			Print();
			exit(ERROR_BADTOKEN);
		};

		states.push_back(BadTokenState);
	}

	void init_num_state()
	{
		State NumTokenState("NumToken");
		NumTokenState.onGuard = [](const char c, Token& t)  -> bool
		{
			if (isdigit(c) || c == '.')
			{
				if (t.type == TOKEN_NUM && c == '.' && t.text.find('.'))
				{
					return false;
				}
				return true;
			}
			return false;
		};


		NumTokenState.onEnter = [](const char c, Token& t) -> void
		{
			t.text += c;
			t.type  = TOKEN_NUM;
		};

		NumTokenState.onExit   = [this](const char c, Token& t) -> void
		{
			
			if (!isdigit(c) && c != '.')
			{
				t.type_str = "Num";
				std::istringstream is(t.text);
				is >> t.value;
				t.text 	   = "";
				tokens.push_back(t);
			}
		};

		states.push_back(NumTokenState);
	}

	void init_add_state()
	{
		State AddTokenState("AddToken");
		AddTokenState.onGuard = [] (const char c, Token& t)  -> bool
		{
			return c == '+';
		};

		AddTokenState.onEnter = [] (const char c, Token& t)  -> void
		{
			t.text += c;
			t.type  = TOKEN_ADD;
		};

		AddTokenState.onExit  = [this] (const char c, Token& t) -> void
		{
			if (c != '+')
			{
				t.type_str = "Add";
				t.text 	   = "";
				tokens.push_back(t);
			}
		};
		states.push_back(AddTokenState);
	}

	void init_sub_state()
	{
		State SubTokenState("SubToken");
		SubTokenState.onGuard = [] (const char c, Token& t)  -> bool
		{
			return c == '-';
		};

		SubTokenState.onEnter = [] (const char c, Token& t)  -> void
		{
			t.text += c;
			t.type  = TOKEN_SUB;
		};

		SubTokenState.onExit  = [this] (const char c, Token& t) -> void
		{
			if (c != '-')
			{
				t.type_str = "Sub";
				t.text 	   = "";
				tokens.push_back(t);
			}
		};
		states.push_back(SubTokenState);
	}

	void init_mul_state()
	{
		State MulTokenState("MulToken");
		MulTokenState.onGuard = [] (const char c, Token& t)  -> bool
		{
			return c == '*';
		};

		MulTokenState.onEnter = [] (const char c, Token& t)  -> void
		{
			t.text += c;
			t.type  = TOKEN_MUL;
		};

		MulTokenState.onExit  = [this] (const char c, Token& t) -> void
		{
			if (c != '*')
			{
				t.type_str = "Mul";
				t.text 	   = "";
				tokens.push_back(t);
			}
		};
		states.push_back(MulTokenState);
	}

	void init_div_state()
	{
		State DivTokenState("DivToken");
		DivTokenState.onGuard = [] (const char c, Token& t)  -> bool
		{
			return c == '/';
		};

		DivTokenState.onEnter = [] (const char c, Token& t)  -> void
		{
			t.text += c;
			t.type  = TOKEN_DIV;
		};

		DivTokenState.onExit  = [this] (const char c, Token& t) -> void
		{
			t.type_str = "Div";
			t.text 	   = "";
			tokens.push_back(t);
		};
		states.push_back(DivTokenState);		
	}

	void init_space_state()
	{
		State SpaceTokenState("SpaceToken");

		SpaceTokenState.onGuard = [] (const char c, Token& t)  -> bool
		{
			return true && isspace(c);
		};
		states.push_back(SpaceTokenState);			
	}

	void init_end_state()
	{
	}

	void init_braket_state()
	{
		State BraketTokenState("BraketToken");

		BraketTokenState.onGuard = [] (const char c, Token& t) -> bool
		{
			std::cout<<"BraketToken"<<std::endl;
			return c == '(' || c == ')';
		};

		BraketTokenState.onEnter = [] (const char c, Token& t) ->void
		{
			t.text += c;
			t.type  = c == '(' ? TOKEN_BRAKET_OPEN : TOKEN_BRAKET_CLOSE;
		};

		BraketTokenState.onExit = [this] (const char c, Token& t) -> void
		{
			t.type_str = t.type == TOKEN_BRAKET_OPEN ? "BraketOpen" : "BraketClose";
			t.text	   = "";
			tokens.push_back(t);
		};
		states.push_back(BraketTokenState);
	}

	std::list<State> states;
	std::list<Token> tokens;
};

int main(int argc, const char** argv)
{

	Lexter lexter;
	lexter.Parse(argv[1]);
	lexter.Print();
	return 0;
}