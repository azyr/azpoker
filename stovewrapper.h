// #include <string>
// #include <vector>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "pokerstove/peval/PokerHandEvaluator.h"
#include "pokerstove/peval/CardSet.h"

using namespace std;
using namespace pokerstove;

int evaluate_high(long long* masks, int numevals, int* out)
{
    // boost::shared_ptr<PokerHandEvaluator> peval = PokerHandEvaluator::alloc('h');

    for (int i = 0; i < numevals; ++i)
    {
        CardSet h(masks[i]);
        // PokerHandEvaluation handval = peval->evaluate(holecards[i], boards[i]);
        // out[i] = handval.high().code();
        out[i] = h.evaluateHigh().code();
    }
    return 0;
}
