/**
 * (c) Mikdore 2021
 * This file is licensed under the Boost Software License
 */
#pragma once
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <type_traits>
#include <stdexcept>

namespace bases
{
	struct options
	{ };

	enum class bases
	{
		BASE16,
		BASE32,
		BASE64
	};
	struct BASEDecodeTable_t
	{
		private:
			std::array<std::uint8_t, 255> table;
		public:

		template<std::size_t N>
		BASEDecodeTable_t(const char (&alphabet)[N]);

		constexpr inline std::uint8_t operator[](char) const noexcept;
	};

	constexpr static char BASE64Alphabet[] 	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	constexpr static char BASE64URLSafe[] 	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	constexpr static auto BASE64Pad			= '=';
	
	static BASEDecodeTable_t BASE64DecodeTable{BASE64Alphabet};





	template<typename T>
	concept StringLike = requires(T t) 
	{
		T();
		t.reserve(std::size_t{});
		t.push_back('a');
	};

	template<typename T>
	concept CharLike = std::is_integral_v<std::remove_reference_t<std::remove_cv_t<T>>> && sizeof(T) == 1;

	template<typename T>
	concept ByteContainerOut = requires(T t)
	{
		T();
		t.reserve(std::size_t{});
		t.push_back(std::uint8_t{});
	};

	template<typename T>
	concept ByteContainerIn = requires(const T& t)
	{
		t.data();
		std::is_pointer_v<decltype(t.data())>;
		t.size();
	} && CharLike<decltype(*std::declval<T>().data())>;

	template<bases Base>
	struct converter	
	{
		static_assert(Base != Base, "Invalid Base");
	};

	template<>
	struct converter<bases::BASE16>
	{
		converter() = delete;

		template<StringLike string_type = std::string>
		static string_type encode(const std::uint8_t*, std::size_t);

		template<ByteContainerOut container_type = std::vector<std::uint8_t>>
		static container_type decode(const char*, std::size_t);	
	};

	template<>
	struct converter<bases::BASE32>
	{
		converter() = delete;

		template<StringLike string_type = std::string>
		static string_type encode(const std::uint8_t*, std::size_t);

		template<ByteContainerOut container_type = std::vector<std::uint8_t>>
		static container_type decode(const char*, std::size_t);
	};

	template<>
	struct converter<bases::BASE64>
	{
		converter() = delete;

		template<StringLike string_type = std::string>
		static string_type encode(const std::uint8_t*, std::size_t);

		template<ByteContainerOut container_type = std::vector<std::uint8_t>>
		static container_type decode(const char*, std::size_t);
	};


	//Raw string literals are not supported by choice

	template<StringLike string_type = std::string, ByteContainerIn container_type>
	string_type b64encode(const container_type& c)
	{
		return converter<bases::BASE64>::encode<string_type>((std::uint8_t*)c.data(), c.size());
	}

	template<ByteContainerOut container_type = std::vector<std::uint8_t>>
	container_type b64decode(std::string_view sv)
	{
		return converter<bases::BASE64>::decode(sv.data(), sv.size());
	}

} // namespace bases

// IMPLEMENTATION

namespace bases
{
	template<std::size_t N>
	BASEDecodeTable_t::BASEDecodeTable_t(const char (&alphabet)[N])
	{
		static_assert(N <= 255 && N > 0, "The alphabet must contain only unique ASCII symbols");
		
		std::fill(std::begin(table), std::end(table), 0xFF);
		table['='] = 0x40; //special - so we can check for it's presence
		for(auto i = 0u; auto c : alphabet)
		{
			table[c] = i;
			i++;
		}
	}
 
	constexpr inline std::uint8_t BASEDecodeTable_t::operator[](char c) const noexcept
	{
		return table[c];
	}


	template<StringLike string_type>
	constexpr inline void BASE64FinalChunk(string_type& out, std::size_t size, std::size_t i, const std::uint8_t* data)
	{
		switch(size)
		{
			case 1: {
				out.push_back( BASE64Alphabet[ 	data[i] 				>> 2 ]);
				out.push_back( BASE64Alphabet[ (data[i] & 0b00000011) 	<< 4 ]);
				out.push_back( BASE64Pad );
				out.push_back( BASE64Pad );
				break;	
			}

			case 2: {
				out.push_back( BASE64Alphabet[ 						data[i] 	>> 2									]);
				out.push_back( BASE64Alphabet[ ((	0b11110000 & 	data[i+1]) 	>> 4) | ((0b00000011 & data[i  ]) << 4) ]);
				out.push_back( BASE64Alphabet[ ((	0b00001111 &	data[i+1]) 	<< 2)									]);
				out.push_back( BASE64Pad );
				break;
			}
			
			case 0: 
			default: break;
		}
	}

	template<StringLike string_type>
	string_type converter<bases::BASE64>::encode(const std::uint8_t* data, std::size_t size)
	{
		string_type out;

		out.reserve( ((4 * size / 3) + 3) & ~3 ); //output length including padding

		if(size < 3)
		{
			BASE64FinalChunk(out, size, 0, data);
			return out;
		}

		auto i = 0ul;

		for(; (size - i) >= 3; i += 3)
		{

			out.push_back( BASE64Alphabet[						data[i	] 	>> 2									]);							
			out.push_back( BASE64Alphabet[ ((	0b11110000 & 	data[i+1]) 	>> 4) | ((0b00000011 & data[i  ]) << 4) ]);
			out.push_back( BASE64Alphabet[ ((	0b00001111 & 	data[i+1]) 	<< 2) | ((0b11000000 & data[i+2]) >> 6) ]);
			out.push_back( BASE64Alphabet[ 		0b00111111 & 	data[i+2]											]);

		}

		BASE64FinalChunk(out, size - i, i, data);

		return out;
	}

	template<ByteContainerOut container_type>
	container_type converter<bases::BASE64>::decode(const char* data, std::size_t size)
	{
		container_type out;
		if(size % 4 != 0)
		{
			throw std::runtime_error{"Bad formatting (size must be multiple of 4)"};
		}
		out.reserve((size/4)*3);
		
		auto i = 0ul;
		
		for(; (size - i) >= 4; i += 4)
		{
			auto a = BASE64DecodeTable[data[i	 ]];
			auto b = BASE64DecodeTable[data[i + 1]];
			auto c = BASE64DecodeTable[data[i + 2]];
			auto d = BASE64DecodeTable[data[i + 3]];

			if((a | b | c | d) & 0x80) //highest bit == bad character
			{
				throw std::runtime_error{"Bad Character"};
			}
			if((a | b | c | d) & 0x40) //pad character == last block
			{
				if(((a | b) & 0x40) || ((c & 0x40) && !(d & 0x40)))
				{
					throw std::runtime_error{"Bad padding"};
				}

				if(c & 0x40)
				{
					out.push_back((a << 2) | (b >> 4));  
					return out;
				} else 
				{
					out.push_back((a 		<< 2) 	| (b >> 4));
					out.push_back((0xf & b) << 4	| (c >> 2));
					return out;
				}
			}

			out.push_back((a << 2) | (b >> 4));
			out.push_back((b << 4) | (c >> 2));
			out.push_back((c << 6) | d);

		}

		return out;

	}



	template<StringLike string_type>
	string_type converter<bases::BASE32>::encode(const std::uint8_t*, std::size_t)
	{
		return "";
	}
	template<StringLike string_type>
	string_type converter<bases::BASE16>::encode(const std::uint8_t*, std::size_t)
	{
		return "";
	}
} // namespace bases