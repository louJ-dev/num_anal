#include <iostream>
#include <string>
#include <sstream>
#include <stack>
#include <cmath>
#include <algorithm>
#include <iomanip>

#if __has_include(<emscripten.h>)

    #include <emscripten.h>
    #include <emscripten/bind.h>

    #define HAS_EMSCRIPTEN 1
#endif


#define BIAS 1023                   // value set for 64-bit 
#define EPSILON 1e-8                // threshold 
#define COMPUTE_PRECISION 1e-12     // used for calculations
#define DISPLAY_PRECISION 10         // used for outputs

std::string decimal_to_binary(double n) {
    if(0 == n) {
        return "0";
    }

    int sign = (n < 0) ? -1 : 1;
    n = std::fabs(n);

    int whole = (int)n;
    double decimal = n - whole;

    std::string result = "";
    while(whole > 0) {
        result.insert(0, 1, '0' + (whole % 2 > 0));
        whole /= 2;
    }

    if(decimal > 0) {
        result += '.';

        while(decimal > 0) {
            decimal *= 2;

            if(decimal >= 1) {
                result += '1';
                decimal -= 1;
            } else {
                result += '0';
            }
        }
    }

    if(-1 == sign) {
        result = '-' + result;
    }

    return result;
}

std::string decimal_to_ieee(double n) {
    if(0 == n) {
        return "0000000000000000000000000000000000000000000000000000000000000000";
    }

    char sign = '0' + (n < 0); 
    std::string binary = decimal_to_binary(std::fabs(n));

    int pointIndex = binary.length();
    for(int i = 0;i<binary.length();i++) {
        if('.' == binary[i]) {
            pointIndex = i;
            break;
        }
    }
    
    int e = (pointIndex > 0) ? pointIndex - 1: -1;
    std::string ebinary = decimal_to_binary(e + BIAS);
    if(ebinary.length() < 11) {
        ebinary.insert(0, 11 - ebinary.length(), '0');
    }

    std::string mantissa = "";
    
    if(binary.length() > 1) {
        binary.erase(pointIndex, 1);
        binary.insert(1, 1, '.');
        mantissa = binary.substr(2);
    }

    if(mantissa.length() > 52) {
        mantissa = mantissa.substr(0, 51);
    } else {
        mantissa.append(52 - mantissa.length(), '0');
    }

    return sign + ebinary + mantissa;
}

std::string decimal_to_ieee_spaced(double n) {
    if(0 == n) {
        return "0 00000000000 0000000000000000000000000000000000000000000000000000";
    }

    char sign = '0' + (n < 0); 
    std::string binary = decimal_to_binary(std::fabs(n));

    int pointIndex = binary.length();
    for(int i = 0;i<binary.length();i++) {
        if('.' == binary[i]) {
            pointIndex = i;
            break;
        }
    }
    
    int e = (pointIndex > 0) ? pointIndex - 1: -1;
    std::string ebinary = decimal_to_binary(e + BIAS);
    if(ebinary.length() < 11) {
        ebinary.insert(0, 11 - ebinary.length(), '0');
    }

    std::string mantissa = "";
    
    if(binary.length() > 1) {
        binary.erase(pointIndex, 1);
        binary.insert(1, 1, '.');
        mantissa = binary.substr(2);
    }

    if(mantissa.length() > 52) {
        mantissa = mantissa.substr(0, 51);
    } else {
        mantissa.append(52 - mantissa.length(), '0');
    }

    std::stringstream ss;
    ss << sign << " " << ebinary << " " << mantissa;
    
    return ss.str();
}

double binary_to_decimal(std::string binary) {
    int i = 0;
    int sign = ('-' == binary[i++]) ? -1 : 1;
    
    while(i < binary.length() && '.' != binary[i]) {
        i++;
    }

    int l = i - 1, mult = 1, whole = 0;
    while(l >= 0 && '-' != binary[l]) {
        whole += (binary[l] - '0') * mult;
        mult *= 2;
        l--;
    }
    
    i++;

    double div = 2.0, decimal = 0;
    while(i < binary.length()) {
        decimal += (binary[i] - '0') * (1.0 / div);
        div *= 2.0;
        i++;
    }

    return (double)sign * ((double)whole + decimal);
}

double ieee_to_decimal(std::string binary) {
    int sign = ('1' == binary[0]) ? -1 : 1;
    int e = (int)binary_to_decimal(binary.substr(1, 11));
    double fraction = 0.0 + ((e != 0) * 1);
    
    double mult = 2.0;
    for(char b : binary.substr(12, 51)) {
        fraction += (b - '0') * (1.0 / mult);
        mult *= 2.0;
    }

    return sign * std::pow(2, (e - BIAS)) * fraction;
}

int get_sig_digits(double rel_error) {
    if(0 == rel_error) {
        return -1;
    }
    
    int sdig = std::floor(1 - std::log10(2*rel_error));

    /*
    while(true) {
        double curr = 5.0 / std::pow(10.0, sdig);

        if(curr >= rel_error && (5.0 / std::pow(10.0, sdig + 1)) < rel_error) {
            break;
        }

        if(curr >= rel_error) {
            sdig++;
        } else {
            sdig--;
        }
    }
    */
    
    return sdig;
}

int get_normalized_exponent(double n) {
    n = std::abs(n);
    
    int exp = 1;

    if(n >= 1) {
        exp = std::floor(std::log10((int)n)) + 1;
    } else if (n < 1) {
        exp = -std::floor(std::log10(n)) - 1;
    } else {
        exp = 0;   
    }

    return exp;
}


double get_chop(double n, int sdigit) {
    if(0.0 == n || !std::isfinite(n) || sdigit < 1) {
        return n;
    }

    /*
    double scale = std::pow(10.0, std::floor(std::log10(std::fabs(n))) - sdigit + 1);
    double scaled = n / scale;
    double chop = std::trunc(scaled);
    double chopped = chop * scale;
    */
    
    // double d = std::ceil(std::log10(std::abs(n)));
    // double power = std::pow(10, sdigit - d);
    
    int exp = get_normalized_exponent(n);
    double power = std::pow(10.0, sdigit + exp);

    // double normalized = n * std::pow(10.0, exp);


    // return std::trunc(normalized * std::pow(10.0, sdigit)) / ( 

    // 2. Shift, truncate, and shift back
    // We use std::trunc to "chop" toward zero
    return std::trunc(n * power) / power;
    // return chopped;
}

double get_chop_unnormalized(double n, int sdigit) {
    if(0.0 == n || !std::isfinite(n)) {
        return n;
    }

    long long int scaled = n * std::pow(10, sdigit);
    double chop = std::trunc(scaled);
    double chopped = chop / std::pow(10, sdigit);

    return chopped;
}

double chop_add_float(double a, double b, int chop) {
    double ca = get_chop(a, chop);
    double cb = get_chop(b, chop); 
    return get_chop(ca + cb, chop);
}

double chop_subtract_float(double a, double b, int chop) {
    double ca = get_chop(a, chop);
    double cb = get_chop(b, chop); 
    return get_chop(ca - cb, chop);
}

double chop_multilpy_float(double a, double b, int chop) {
    double ca = get_chop(a, chop);
    double cb = get_chop(b, chop); 
    return get_chop(ca * cb, chop);
}

double chop_divide_float(double a, double b, int chop) {
    double ca = get_chop(a, chop);
    double cb = get_chop(b, chop); 
    return get_chop(ca / cb, chop);
}

double chop_pow(double b, int pow, int chop) {
    return get_chop(std::pow(b, pow), chop);
}

double chop_sqrts(double n, int chop) {
    return get_chop(std::sqrt(n), chop);
}


/* BUG: rounding is not implemented properly
 *      floating-point numbers cannot be truely be rounded
 */

double get_round(double n, int place) {
    /*
    long long int convert = n * std::pow(10, place);
    if(convert % 10 >= 5) {
        convert += 10;
    }

    double r = convert / std::pow(10.0, place);
    */

    if(n == 0) {
        return 0;
    }
    
    int s = get_normalized_exponent(n);
    
    double r = n + (5*std::pow(10.0, s-(place+1)));
    r = get_chop_unnormalized(r, place - 1);
    /*
    
    if(std::abs(n - r) <= DISPLAY_PRECISION) {
        return r; 
    }
    
    */
    

    return r;
}

double get_round_normalized(double n, int place) {
    /*
    long long int convert = n * std::pow(10, place);
    if(convert % 10 >= 5) {
        convert += 10;
    }
*/

    if(n == 0) {
        return 0;
    }
    
    int s = get_normalized_exponent(n);
    
    double r = (n * std::pow(10.0, s)) + (5 * std::pow(10.0, -(place + 1)));
    r *= std::pow(10.0, -s);
    r = get_chop_unnormalized(r, place);
    /*
    
    if(std::abs(n - r) <= DISPLAY_PRECISION) {
        return r; 
    }
    
    */
    

    return r;
}

bool is_digit(char c) {
    if(c >= '0' && c <= '9') {
        return true;
    }

    return false;
}

bool is_operator(char c) {
    switch(c) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '%':
            return true;
        default:
            return false;
    }
}

int get_precedence(char operation) {
    if(operation == '+' || operation == '-') {
        return 2;
    } else if(operation == '*' || operation == '/') {
        return 3;
    } else if(operation == '^') {
        return 4;
    } else {
        return -1;
    }
}

/*  
 *
 *  return: -1 for left-associative
 *          +1 for right-associative
 *          0  for unknown;
 *
 */

int get_associative(char operation) {
    switch(operation){
        case '+': 
        case '-': 
        case '*': 
        case '/':
            return -1;
        case '^':
            return 1;
        default:
            return 0;
    }
}

// returns: first whole num from index (i) 
//          stops until non-numeric char
// returns: zero when no number is found
double get_num_from_index(int i, std::string s, int *length) { 
    int sign = 1;
    int len = 0;

    if(s[i] == '-') {
        sign = -1;
        len++;
        i++;
    }

    double num = 0;
    while(i < s.length() && is_digit(s[i])) {
        num = (num * 10) + (s[i] - '0');
        len++;
        i++;   
    }
    
    if(i < s.length() && '.' == s[i]) {
        i++;
        len++;
        
        double placement = 0.1;
        
        while(i < s.length() && is_digit(s[i])) {
            num += ((double)s[i] - '0') * placement;
            placement /= 10;
            
            i++;
            len++;
        }
    }

    *length = len;

    return num * sign;
}

double evaluate(double a, double b, char oper) {
    double result;

    if('+' == oper) {     
        result = a + b;
    } else if('-' == oper) {
        result = a - b;
    } else if('*' == oper) {
        result = a * b;
    } else if('/' == oper) {
        if(b == 0) {
            throw std::runtime_error("Division by Zero");
        }

        result = a / b;
    } else if('^' == oper){
        result = std::pow(a, b);
    } else {
        throw std::runtime_error("Invalid operation");
    }

    return result;
}

double evaluate_chop(double a, double b, char oper, int sig) {
    a = get_chop(a, sig);
    b = get_chop(b, sig);

    double result = evaluate(a, b, oper);
    
    std::cout << result << " -> " << get_chop(result, sig) << '\n';

    return get_chop(result, sig);
}

double evaluate_round(double a, double b, char oper, int place) {
    a = get_round_normalized(a, place);
    b = get_round_normalized(b, place);

    double result = evaluate(a, b, oper);
    
    return get_round_normalized(result, place);
}

void remove_whitespace(std::string& str) {
    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end()); 
}

void replace_all(std::string& str, const std::string& from, const std::string& to) { 
    if(from.empty()) {
        return;
    }

    size_t pos = str.find(from, 0);
    while(pos != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
        pos = str.find(from, 0);
    }
}

void replace_constants(std::string& exp) {
    const std::string PI_REP = "3.14159265359";
    const std::string E_REP  = "2.7182818284";
    
    replace_all(exp, "PI", PI_REP); 
    replace_all(exp, "pi", PI_REP); 

    replace_all(exp, "E", E_REP); 
    replace_all(exp, "e", E_REP); 
}

void solve_expression_helper(std::stack<double>& nums, std::stack<char>& opers) {
    if(nums.size() < 2) {
        throw std::runtime_error("Missing operand");
    }

    if(opers.empty() || opers.top() == '(') {
        throw std::runtime_error("Invalid expression");
    }

    double b = nums.top();
    nums.pop();
    double a = nums.top();
    nums.pop();
    char o = opers.top();
    opers.pop();
    nums.push(evaluate(a, b, o));
}

double solve_expression(std::string expr) {
    remove_whitespace(expr);
    replace_constants(expr);

    std::stack<double> nums;
    std::stack<char> opers;
    
    int i = 0;
    
    // Removed the buggy expr[0] check. The while-loop now natively handles
    // unary operators at the start of the string.

    while(i < expr.length()) {
        // digit 
        if(is_digit(expr[i])) {
            int num_len;
            nums.push(get_num_from_index(i, expr, &num_len));
            i += num_len - 1;
        } 

        // operator (+, -, *, /, ^)
        else if(is_operator(expr[i])) {
            
            // 1. Unary Plus (e.g., "+5" or "(+5)"): completely ignore it
            if(expr[i] == '+' && (i == 0 || is_operator(expr[i-1]) || expr[i-1] == '(')) {
                i++;
                continue; 
            }
            
            // 2. Unary Minus (e.g., "-5" or "-(5)")
            else if(expr[i] == '-' && (i == 0 || is_operator(expr[i-1]) || expr[i-1] == '(')) {
                if(i + 1 < expr.length() && is_digit(expr[i+1])) {
                    // Negative number attached directly to digits
                    int num_len;
                    nums.push(get_num_from_index(i, expr, &num_len));
                    i += num_len - 1;
                } else {
                    // Unary minus before a parenthesis, e.g., -(5)
                    // Treat it mathematically as "-1 *" 
                    nums.push(-1.0);
                    
                    int preced_curr = get_precedence('*');
                    int preced_top = -1;
                    while(!opers.empty() 
                            && opers.top() != '(' 
                            // FIX: Added parentheses around the assignment
                            && ((preced_top = get_precedence(opers.top())) > preced_curr
                                || (preced_curr == preced_top && get_associative('*') == -1))) {
                        solve_expression_helper(nums, opers); 
                    }
                    opers.push('*');
                }
            } 
            
            // 3. Normal Binary Operator
            else {
                int preced_curr = get_precedence(expr[i]);
                int preced_top = -1;
                while(!opers.empty() 
                        && opers.top() != '(' 
                        // FIX: Added parentheses around the assignment
                        && ((preced_top = get_precedence(opers.top())) > preced_curr
                            || (preced_curr == preced_top && get_associative(expr[i]) == -1))) {
                    solve_expression_helper(nums, opers); 
                }
                opers.push(expr[i]);
            }
        } 

        // left parenthesis
        else if(expr[i] == '(') {
            
            // FIX: Handle implicit multiplication (e.g., "5(" or ")(") safely
            if(i > 0 && (is_digit(expr[i-1]) || ')' == expr[i-1])) {
                int preced_curr = get_precedence('*');
                int preced_top = -1;
                
                // Process the implicit '*' before pushing the '('
                while(!opers.empty() 
                        && opers.top() != '(' 
                        && ((preced_top = get_precedence(opers.top())) > preced_curr
                            || (preced_curr == preced_top && get_associative('*') == -1))) {
                    solve_expression_helper(nums, opers); 
                }
                opers.push('*');
            }
            
            opers.push('(');
        } 

        // right parenthesis
        else if(expr[i] == ')') {
            while(!opers.empty() && opers.top() != '(') {
                solve_expression_helper(nums, opers); 
            }
            
            // Added check to prevent popping an empty stack on mismatched parentheses
            if(!opers.empty() && opers.top() == '(') {
                opers.pop();
            } else {
                throw std::runtime_error("Mismatched parentheses");
            }
        } 

        i++;
    }

    while(!opers.empty()) {
        solve_expression_helper(nums, opers); 
    }
    
    // FIX: Catch empty expressions to prevent calling .top() on an empty stack
    if (nums.empty()) {
        throw std::runtime_error("Empty or invalid expression");
    }
    
    return get_round(nums.top(), DISPLAY_PRECISION); 
}





/*
 *
 *
 *  CHOP
 *
 *
 */





void solve_expression_helper_chop(std::stack<double>& nums, std::stack<char>& opers, int chop) {
    if(nums.size() < 2) {
        throw std::runtime_error("Missing operand");
    }

    if(opers.empty() || opers.top() == '(') {
        throw std::runtime_error("Invalid expression");
    }

    double b = nums.top();
    nums.pop();
    double a = nums.top();
    nums.pop();
    char o = opers.top();
    opers.pop();

    nums.push(evaluate_chop(a, b, o, chop));
}

double solve_expression_chop(std::string expr, int digit) {
    remove_whitespace(expr);
    replace_constants(expr);

    std::stack<double> nums;
    std::stack<char> opers;
    
    int i = 0;
    
    // Removed the buggy expr[0] check. The while-loop now natively handles
    // unary operators at the start of the string.

    while(i < expr.length()) {
        // digit 
        if(is_digit(expr[i])) {
            int num_len;
            nums.push(get_num_from_index(i, expr, &num_len));
            i += num_len - 1;
        } 

        // operator (+, -, *, /, ^)
        else if(is_operator(expr[i])) {
            
            // 1. Unary Plus (e.g., "+5" or "(+5)"): completely ignore it
            if(expr[i] == '+' && (i == 0 || is_operator(expr[i-1]) || expr[i-1] == '(')) {
                i++;
                continue; 
            }
            
            // 2. Unary Minus (e.g., "-5" or "-(5)")
            else if(expr[i] == '-' && (i == 0 || is_operator(expr[i-1]) || expr[i-1] == '(')) {
                if(i + 1 < expr.length() && is_digit(expr[i+1])) {
                    // Negative number attached directly to digits
                    int num_len;
                    nums.push(get_num_from_index(i, expr, &num_len));
                    i += num_len - 1;
                } else {
                    // Unary minus before a parenthesis, e.g., -(5)
                    // Treat it mathematically as "-1 *" 
                    nums.push(-1.0);
                    
                    int preced_curr = get_precedence('*');
                    int preced_top = -1;
                    while(!opers.empty() 
                            && opers.top() != '(' 
                            // FIX: Added parentheses around the assignment
                            && ((preced_top = get_precedence(opers.top())) > preced_curr
                                || (preced_curr == preced_top && get_associative('*') == -1))) {
                        solve_expression_helper_chop(nums, opers, digit); 
                    }
                    opers.push('*');
                }
            } 
            
            // 3. Normal Binary Operator
            else {
                int preced_curr = get_precedence(expr[i]);
                int preced_top = -1;
                while(!opers.empty() 
                        && opers.top() != '(' 
                        // FIX: Added parentheses around the assignment
                        && ((preced_top = get_precedence(opers.top())) > preced_curr
                            || (preced_curr == preced_top && get_associative(expr[i]) == -1))) {
                    solve_expression_helper_chop(nums, opers, digit); 
                }
                opers.push(expr[i]);
            }
        } 

        // left parenthesis
        else if(expr[i] == '(') {
            
            // FIX: Handle implicit multiplication (e.g., "5(" or ")(") safely
            if(i > 0 && (is_digit(expr[i-1]) || ')' == expr[i-1])) {
                int preced_curr = get_precedence('*');
                int preced_top = -1;
                
                // Process the implicit '*' before pushing the '('
                while(!opers.empty() 
                        && opers.top() != '(' 
                        && ((preced_top = get_precedence(opers.top())) > preced_curr
                            || (preced_curr == preced_top && get_associative('*') == -1))) {
                    solve_expression_helper_chop(nums, opers, digit); 
                }
                opers.push('*');
            }
            
            opers.push('(');
        } 

        // right parenthesis
        else if(expr[i] == ')') {
            while(!opers.empty() && opers.top() != '(') {
                solve_expression_helper_chop(nums, opers, digit); 
            }
            
            // Added check to prevent popping an empty stack on mismatched parentheses
            if(!opers.empty() && opers.top() == '(') {
                opers.pop();
            } else {
                throw std::runtime_error("Mismatched parentheses");
            }
        } 

        i++;
    }

    while(!opers.empty()) {
        solve_expression_helper_chop(nums, opers, digit); 
    }
    
    // FIX: Catch empty expressions to prevent calling .top() on an empty stack
    if (nums.empty()) {
        throw std::runtime_error("Empty or invalid expression");
    }
    
    return get_chop_unnormalized(nums.top(), digit); 
}



/*
 *
 *
 *
 *  ROUND
 *
 *
 *
 */

void solve_expression_helper_round(std::stack<double>& nums, std::stack<char>& opers, int digit) {
    if(nums.size() < 2) {
        throw std::runtime_error("Missing operand");
    }

    if(opers.empty() || opers.top() == '(') {
        throw std::runtime_error("Invalid expression");
    }

    double b = nums.top();
    nums.pop();
    double a = nums.top();
    nums.pop();
    char o = opers.top();
    opers.pop();

    nums.push(evaluate_round(a, b, o, digit));
}

double solve_expression_round(std::string expr, int digit) {
    remove_whitespace(expr);
    replace_constants(expr);

    std::stack<double> nums;
    std::stack<char> opers;
    
    int i = 0;
    
    // Removed the buggy expr[0] check. The while-loop now natively handles
    // unary operators at the start of the string.

    while(i < expr.length()) {
        // digit 
        if(is_digit(expr[i])) {
            int num_len;
            nums.push(get_num_from_index(i, expr, &num_len));
            i += num_len - 1;
        } 

        // operator (+, -, *, /, ^)
        else if(is_operator(expr[i])) {
            
            // 1. Unary Plus (e.g., "+5" or "(+5)"): completely ignore it
            if(expr[i] == '+' && (i == 0 || is_operator(expr[i-1]) || expr[i-1] == '(')) {
                i++;
                continue; 
            }
            
            // 2. Unary Minus (e.g., "-5" or "-(5)")
            else if(expr[i] == '-' && (i == 0 || is_operator(expr[i-1]) || expr[i-1] == '(')) {
                if(i + 1 < expr.length() && is_digit(expr[i+1])) {
                    // Negative number attached directly to digits
                    int num_len;
                    nums.push(get_num_from_index(i, expr, &num_len));
                    i += num_len - 1;
                } else {
                    // Unary minus before a parenthesis, e.g., -(5)
                    // Treat it mathematically as "-1 *" 
                    nums.push(-1.0);
                    
                    int preced_curr = get_precedence('*');
                    int preced_top = -1;
                    while(!opers.empty() 
                            && opers.top() != '(' 
                            // FIX: Added parentheses around the assignment
                            && ((preced_top = get_precedence(opers.top())) > preced_curr
                                || (preced_curr == preced_top && get_associative('*') == -1))) {
                        solve_expression_helper_round(nums, opers, digit); 
                    }
                    opers.push('*');
                }
            } 
            
            // 3. Normal Binary Operator
            else {
                int preced_curr = get_precedence(expr[i]);
                int preced_top = -1;
                while(!opers.empty() 
                        && opers.top() != '(' 
                        // FIX: Added parentheses around the assignment
                        && ((preced_top = get_precedence(opers.top())) > preced_curr
                            || (preced_curr == preced_top && get_associative(expr[i]) == -1))) {
                    solve_expression_helper_round(nums, opers, digit); 
                }
                opers.push(expr[i]);
            }
        } 

        // left parenthesis
        else if(expr[i] == '(') {
            
            // FIX: Handle implicit multiplication (e.g., "5(" or ")(") safely
            if(i > 0 && (is_digit(expr[i-1]) || ')' == expr[i-1])) {
                int preced_curr = get_precedence('*');
                int preced_top = -1;
                
                // Process the implicit '*' before pushing the '('
                while(!opers.empty() 
                        && opers.top() != '(' 
                        && ((preced_top = get_precedence(opers.top())) > preced_curr
                            || (preced_curr == preced_top && get_associative('*') == -1))) {
                    solve_expression_helper_round(nums, opers, digit); 
                }
                opers.push('*');
            }
            
            opers.push('(');
        } 

        // right parenthesis
        else if(expr[i] == ')') {
            while(!opers.empty() && opers.top() != '(') {
                solve_expression_helper_round(nums, opers, digit); 
            }
            
            // Added check to prevent popping an empty stack on mismatched parentheses
            if(!opers.empty() && opers.top() == '(') {
                opers.pop();
            } else {
                throw std::runtime_error("Mismatched parentheses");
            }
        } 

        i++;
    }

    while(!opers.empty()) {
        solve_expression_helper_round(nums, opers, digit); 
    }
    
    // FIX: Catch empty expressions to prevent calling .top() on an empty stack
    if (nums.empty()) {
        throw std::runtime_error("Empty or invalid expression");
    }
    
    return get_round(nums.top(), digit); 
}











/*
 *
 * ERROR ANALYSIS:
 * 
 * get_absolute_error();
 * get_relative_error();
 * get_sig_digits_error();
 * get_max_abs_error();
 *
 *
*/

double get_absolute_error(double exact, double approximate) {
    return std::abs(exact - approximate);
}

double get_relative_error(double exact, double approximate) {
    return std::abs(exact - approximate) / std::abs(exact);
}

double get_max_abs_error(double exact, int sig_digits) {
    return 5.0 * std::pow(10.0, -sig_digits) * std::fabs(exact);
}



/*
 *
 *  allows c++ to work in js as a connection
 *
 *  add functions so that js can access it
 *
 *  change string parameter to change function name in js
 *
 *
*/

std::string get_all_errors(double e, double a) {
    double abs_err = get_absolute_error(e, a);
    double rel_err = get_relative_error(e, a);
    int sig_dig = get_sig_digits(rel_err);
    double mabs_err = get_max_abs_error(e, sig_dig);


    std::stringstream ss;
    ss << std::fixed << std::setprecision(8);
    ss << "Absolute Error: " << abs_err << '\n';
    ss << "Relative Error: " << rel_err << '\n';
    ss << "Significant Digits: " << (sig_dig == -1 ? "Exact and Approximate are equal" : std::to_string(sig_dig)) << '\n';
    ss << "Max Absolute Error: " << (sig_dig == -1 ? "Exact and Approximate are equal" : std::to_string(mabs_err)) << '\n';

    return ss.str();
}

#if HAS_EMSCRIPTEN

    EMSCRIPTEN_BINDINGS(my_module) {
        emscripten::function("decimal_to_binary", &decimal_to_binary);
        emscripten::function("binary_to_decimal", &binary_to_decimal);
        emscripten::function("ieee_to_decimal", &ieee_to_decimal);
        emscripten::function("decimal_to_ieee", &decimal_to_ieee);
        emscripten::function("decimal_to_ieee_spaced", &decimal_to_ieee_spaced);
        emscripten::function("solve_expression", &solve_expression);

        emscripten::function("solve_expression_chop", &solve_expression_chop);
        emscripten::function("solve_expression_round", &solve_expression_round);
        emscripten::function("get_chop", &get_chop);
        emscripten::function("get_round", &get_round);
        emscripten::function("get_absolute_error", &get_absolute_error);
        emscripten::function("get_relative_error", &get_relative_error);
        emscripten::function("get_sig_digits", &get_sig_digits);
        emscripten::function("get_max_abs_error", &get_max_abs_error);
        emscripten::function("get_all_errors", &get_all_errors);
    }

#endif


int main() {
    std::cout << std::fixed << std::setprecision(10);
    std::cout << solve_expression_chop("(((5/7)-(0.714251))*(98765.9))+((1/3)/(0.111111))", 5);

    return 0; 
}
