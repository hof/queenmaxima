#define BOOST_TEST_MODULE fenton
#include <boost/test/included/unit_test.hpp>

#include "fast.h"
#include "parser.h"
#include "main.h"

TMainForm MainForm;

BOOST_AUTO_TEST_CASE(fenton1)
{
	TFastNode *node = new TFastNode();
	fenton(node,"rnbqkbnr/pppppppp/////PPPPPPPP/RNBQKBNR w kqKQ");

    BOOST_CHECK_EQUAL(node->wpawns, 8);
}
