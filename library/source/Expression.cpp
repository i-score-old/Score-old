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


TTAddress& Expression::getAddress()
{
    return mAddress;
}

TTSymbol& Expression::getOperator()
{
    return mOperator;
}

TTValue& Expression::getValue()
{
    return mValue;
}

TTBoolean Expression::evaluate(const TTValue& value)
{
    if (mOperator == kTTSymEmpty)
        return YES;
    
    if (mOperator == TTSymbol("=="))
        return mValue == value;
    
    if (mOperator == TTSymbol("!="))
        return !(mValue == value);
    
    if (mOperator == TTSymbol(">"))
        return mValue > value;
    
    if (mOperator == TTSymbol(">="))
        return mValue >= value;
    
    if (mOperator == TTSymbol("<"))
        return mValue < value;
    
    if (mOperator == TTSymbol("<="))
        return mValue <= value;
    
    return NO;
}

void Expression::parse(TTValue& toParse)
{
    if (toParse.size() > 0) {
        
        if (toParse[0].type() == kTypeSymbol) {
            
            mAddress = toParse[0];
            
            if (toParse.size() > 1) {
                
                if (toParse[1].type() == kTypeSymbol) {
                    
                    mOperator = toParse[1];
                    
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
    // copy to protect the value
    TTValue v = toParse;
    
    v.toString();
    
    anExpression = Expression(TTString(v[0]));
}