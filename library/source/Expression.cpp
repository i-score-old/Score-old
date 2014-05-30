/** @file
 *
 * @ingroup scoreLibrary
 *
 * @brief an Expression is a parsed symbol containing an address a logical operator and a value
 *
 * @see TTTimeCondition
 *
 * @authors Théo de la Hogue & Clément Bossut
 *
 * @copyright Copyright © 2013, Théo de la Hogue & Clément Bossut @n
 * This code is licensed under the terms of the "CeCILL-C" @n
 * http://www.cecill.info
 */

#include "Expression.h"


const TTAddress& Expression::getAddress() const
{
    return mAddress;
}

const TTSymbol& Expression::getOperator() const
{
    return mOperator;
}

const TTValue& Expression::getValue() const
{
    return mValue;
}

TTBoolean Expression::evaluate(const TTValue& value)
{
    if (mOperator == kTTSymEmpty)
        return YES;
    
    if (mOperator == TTSymbol("equal"))
        return value == mValue;
    
    if (mOperator == TTSymbol("different"))
        return value != mValue;
    
    if (mOperator == TTSymbol("greaterThan"))
        return value > mValue;
    
    if (mOperator == TTSymbol("greaterThanOrEqual"))
        return value >= mValue;
    
    if (mOperator == TTSymbol("lowerThan"))
        return value < mValue;
    
    if (mOperator == TTSymbol("lowerThanOrEqual"))
        return value <= mValue;
    
    return NO;
}

void Expression::parse(TTValue& toParse)
{
    // parse address
    if (toParse.size() > 0) {
        
        if (toParse[0].type() == kTypeSymbol) {
            
            mAddress = toParse[0];
            
            // parse operator
            if (toParse.size() > 1) {
                
                if (toParse[1].type() == kTypeSymbol) {
                    
                    mOperator = toParse[1];
                    
                    // we need to use word instead of sign because < and > symbol make trouble for XmlFormat parsing
                    if (mOperator == TTSymbol("=="))
                        mOperator = TTSymbol("equal");
                    
                    else if (mOperator == TTSymbol("!="))
                        mOperator = TTSymbol("different");
                    
                    else if (mOperator == TTSymbol(">"))
                        mOperator = TTSymbol("greaterThan");
                    
                    else if (mOperator == TTSymbol(">="))
                        mOperator = TTSymbol("greaterThanOrEqual");
                    
                    else if (mOperator == TTSymbol("<"))
                        mOperator = TTSymbol("lowerThan");
                    
                    else if (mOperator == TTSymbol("<="))
                        mOperator = TTSymbol("lowerThanOrEqual");
                    
                    // parse value
                    if (toParse.size() > 2) {
                        
                        mValue.copyFrom(toParse, 2);
                    }
                }
            }
        }
    }
}

#if 0
#pragma mark -
#pragma mark Some Methods
#endif

void TTSCORE_EXPORT ExpressionParseFromValue(const TTValue& toParse, Expression& anExpression)
{
    // if the expression is composed by one symbol
    if (toParse.size() == 1 && toParse[0].type() == kTypeSymbol) {
        
        TTSymbol s = toParse[0];
        anExpression = Expression(s.c_str());
    }
    else {
        
        // copy to protect the value
        TTValue v = toParse;
        v.toString();
        anExpression = Expression(TTString(v[0]));
    }
}
