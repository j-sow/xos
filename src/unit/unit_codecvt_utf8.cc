#include "codecvt/codecvt"
#include "unit_codecvt_base.h"

static int utf8_chars_needed(unsigned long char_value)
{
	int ret = -1;
	if (char_value < 0x80)
		ret = 1;
	else if (char_value < 0x800)
		ret = 2;
	else if (char_value < 0x10000)
		ret = 3;
	else if (char_value < 0x2000000)
		ret = 4;
	else if (char_value < 0x4000000)
		ret = 5;
	else if (char_value < 0x80000000)
		ret = 6;
	return ret;
}

template <typename C, unsigned long N, int M>
class Test_codecvt_utf8
  : public Test_codecvt_base<std::codecvt_utf8<C, N, (std::codecvt_mode)M>>
{
	typedef Test_codecvt_base<
	          std::codecvt_utf8<C, N, (std::codecvt_mode)M>> base;

	typedef C ictype;

	CPPUNIT_TEST_SUITE(Test_codecvt_utf8);
	CPPUNIT_TEST(construction);
	CPPUNIT_TEST(encoding);
	CPPUNIT_TEST(always_noconv);
	CPPUNIT_TEST(max_length);
	CPPUNIT_TEST(encode_decode_char_range);
	CPPUNIT_TEST_SUITE_END();

 public:
	virtual ~Test_codecvt_utf8() { }


	void max_length() override
	{
		typename base::cvt_t cvt;
		unsigned long max_value =
		  std::min(N,
		    static_cast<unsigned long>(std::numeric_limits<C>::max()));

		std::codecvt_mode m = static_cast<std::codecvt_mode>(M);

		if (m & std::consume_header)
		{
			CPPUNIT_ASSERT(
			  (utf8_chars_needed(max_value) +
			   utf8_chars_needed(bom_value()) == cvt.max_length()));
		} else
		{
			CPPUNIT_ASSERT(utf8_chars_needed(max_value) == cvt.max_length());
		}
	}

	void encode_decode_char_range() override
	{
		typename base::cvt_t cvt;
		unsigned long max_value =
		  std::min(N,
		    static_cast<unsigned long>(std::numeric_limits<C>::max()));

		const int obufsz = 10;
		char outbuffer[obufsz];
		
		std::codecvt_base::result rc = std::codecvt_base::ok;

#if 0
		if (  (N < static_cast<unsigned long>(bom_value()))
		   && (static_cast<std::codecvt_mode>(M) & std::generate_header))
		{
			out_gives_error();
			return;
		}
#endif

		unsigned long cval = 0;
		for (; cval <= std::min(0xd7fful, max_value); ++cval)
		{
			char * end = outbuffer + obufsz;
			rc = out_for_ictype(cval, outbuffer, end);

			CPPUNIT_ASSERT( (rc == std::codecvt_base::ok)
			  || ( (N < static_cast<unsigned long>(bom_value()))
			     && (static_cast<std::codecvt_mode>(M) & std::generate_header)
			     && (rc == std::codecvt_base::error)) );
		}

		for (cval = 0xe000ul; cval <= std::min(0x10fffful, max_value); ++cval)
		{
			char * end = outbuffer + obufsz;
			rc = out_for_ictype(cval, outbuffer, end);
			CPPUNIT_ASSERT(rc == std::codecvt_base::ok);
		}

		for (cval = 0x110000ul; cval <= max_value; cval += 0x10000ul)
		{
			char * end = outbuffer + obufsz;
			rc = out_for_ictype(cval - 1, outbuffer, end);
			CPPUNIT_ASSERT(rc == std::codecvt_base::ok);

			/////

			end = outbuffer + obufsz;
			rc = out_for_ictype(cval, outbuffer, end);
			CPPUNIT_ASSERT(rc == std::codecvt_base::ok);

			/////

			end = outbuffer + obufsz;
			rc = out_for_ictype(cval + 1, outbuffer, end);
			CPPUNIT_ASSERT(rc == std::codecvt_base::ok);
		}
	}

 private:
	std::codecvt_base::result
	out_for_ictype(ictype c, char * buffer, char *& buffer_end)
	{
		typename base::cvt_t cvt;
		std::codecvt_base::result rc = std::codecvt_base::ok;
		std::mbstate_t state = {0, {0}};
		ictype internarray[2] = { c , 0 };
		const ictype * from_next = nullptr;
		char * to_next = nullptr;
		rc = cvt.out(state, internarray, internarray + 1, from_next,
		             buffer, buffer_end, to_next);

		CPPUNIT_ASSERT(  (rc == std::codecvt_base::error)
		              || (to_next <= buffer_end));
		CPPUNIT_ASSERT(  (rc == std::codecvt_base::error)
		              || (from_next == internarray + 1));

		return rc;
	}
};

#define REGISTER_CVT_UTF8(C, N, M) \
	static CppUnit::AutoRegisterSuite< \
		Test_codecvt_utf8<C, N, M>> \
	autoRegisterRegistry__CVTUTF8_ ## C ## N ## M;

#define REGISTER_CVT_UTF8_ALL_MODES(C, MAX) \
	REGISTER_CVT_UTF8(C, MAX, 0) \
	REGISTER_CVT_UTF8(C, MAX, 1) \
	REGISTER_CVT_UTF8(C, MAX, 2) \
	REGISTER_CVT_UTF8(C, MAX, 3) \
	REGISTER_CVT_UTF8(C, MAX, 4) \
	REGISTER_CVT_UTF8(C, MAX, 5) \
	REGISTER_CVT_UTF8(C, MAX, 6) \
	REGISTER_CVT_UTF8(C, MAX, 7)

REGISTER_CVT_UTF8_ALL_MODES(wchar_t, 0x7f);
REGISTER_CVT_UTF8_ALL_MODES(wchar_t, 0xff);
REGISTER_CVT_UTF8_ALL_MODES(wchar_t, 0xffff);

REGISTER_CVT_UTF8_ALL_MODES(char16_t, 0x7f);
REGISTER_CVT_UTF8_ALL_MODES(char16_t, 0xff);
REGISTER_CVT_UTF8_ALL_MODES(char16_t, 0xffff);

REGISTER_CVT_UTF8_ALL_MODES(char32_t, 0x7f);
REGISTER_CVT_UTF8_ALL_MODES(char32_t, 0xff);
REGISTER_CVT_UTF8_ALL_MODES(char32_t, 0xffff);
REGISTER_CVT_UTF8_ALL_MODES(char32_t, 0x10ffff);
REGISTER_CVT_UTF8_ALL_MODES(char32_t, 0x3ffffff);
REGISTER_CVT_UTF8_ALL_MODES(char32_t, 0x7fffffff);