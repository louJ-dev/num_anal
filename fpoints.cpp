#include<bits/stdc++.h>

std::string decimal_to_binary(long double n) {
    if(0 == n) {
        return "0";
    }

    int sign = 1;

    if(n < 0) sign = -1;

    n = std::fabs(n);

    int whole = (int)n;
    long double decimal = n - whole;

    std::string result = "";
    while(whole > 0) {
        if(whole % 2 > 0) {
            result = '1' + result;
        } else {
            result = '0' + result;
        }

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

    if(sign == -1) {
        result = '-' + result;
    }

    return result;
}

std::string decimal_to_ieee(long double n) {
    if(0 == n) {
        return "0000000000000000000000000000000000000000000000000000000000000000";
    }

    std::string sign = "0";
    if(n < 0) sign = "1";
    
    std::string binary = decimal_to_binary(std::fabs(n));

    int pointIndex = binary.length();
    for(int i = 0;i<binary.length();i++) {
        if('.' == binary[i]) {
            pointIndex = i;
            break;
        }
    }
    
    int e = (pointIndex > 0) ? pointIndex - 1: -1;
    std::string ebinary = decimal_to_binary(e + 1023);
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

long double binary_to_decimal(std::string binary) {
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

    long double div = 2.0, decimal = 0;
    while(i < binary.length()) {
        decimal += (binary[i] - '0') * (1.0 / div);
        div *= 2.0;
        i++;
    }

    return (long double)sign * ((long double)whole + decimal);
}

long double ieee_to_decimal(std::string binary) {
    int sign = ('1' == binary[0]) ? -1 : 1;
    int e = (int)binary_to_decimal(binary.substr(1, 11));
    long double fraction = 0.0 + ((e != 0) * 1);
    
    long double mult = 2.0;
    for(char b : binary.substr(12, 51)) {
        fraction += (b - '0') * (1.0 / mult);
        mult *= 2.0;
    }

    return sign * std::pow(2, (e - 1023)) * fraction;
}

long double get_chop(long double n, int sdigit) {
    if(0.0 == n || !std::isfinite(n)) {
        return n;
    }

    long double scale = std::pow(10.0, std::floor(std::log10(std::fabs(n))) - sdigit + 1);
    long double scaled = n / scale;
    long double chop = std::trunc(scaled);
    long double chopped = chop * scale;

    return chopped;
}

long double chop_add_float(long double a, long double b, int chop) {
    long double ca = get_chop(a, chop);
    long double cb = get_chop(b, chop); 
    return get_chop(ca + cb, chop);
}

long double chop_subtract_float(long double a, long double b, int chop) {
    long double ca = get_chop(a, chop);
    long double cb = get_chop(b, chop); 
    return get_chop(ca - cb, chop);
}

long double chop_multilpy_float(long double a, long double b, int chop) {
    long double ca = get_chop(a, chop);
    long double cb = get_chop(b, chop); 
    return get_chop(ca + cb, chop);
}

long double chop_divide_float(long double a, long double b, int chop) {
    long double ca = get_chop(a, chop);
    long double cb = get_chop(b, chop); 
    return get_chop(ca / cb, chop);
}

long double chop_pow(long double b, int pow, int chop) {
    return get_chop(std::pow(b, pow), chop);
}

long double chop_sqrts(long double n, int chop) {
    return get_chop(std::sqrt(n), chop);
}

bool is_digit(char c) {
    if(c >= '0' && c <= '9') {
        return true;
    }

    return false;
}

bool is_operator(char c) {
    if(c == '+' ||
        c == '-' ||
        c == '/' ||
        c == '*' ||
        c == '^' ||
        c == '%') {
        return true;    
    }

    return false;
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


/* TODO: fix function to read decimals
 *
*/

// returns: first whole num from index (i) 
//          stops until non-numeric char
// returns: zero when no number is found
long double get_num_from_index(int i, std::string s, int *length) { 
    int sign = 1;
    int len = 0;

    if(s[i] == '-') {
        sign = -1;
        len++;
        i++;
    }

    long double num = 0;
    while(i < s.length() && is_digit(s[i])) {
        num = (num * 10) + (s[i] - '0');
        len++;
        i++;   
    }
    
    if(i < s.length() && '.' == s[i]) {
        i++;
        len++;
        
        long double placement = 0.1;
        
        while(i < s.length() && is_digit(s[i])) {
            num += ((long double)s[i] - '0') * placement;
            placement /= 10;
            
            i++;
            len++;
        }
    }

    *length = len;

    return num * sign;
}

bool try_evaluate(long double a, long double b, char operation, long double* result) {
    if('+' == operation) {
        *result = a + b;
        return true;
    } else if('-' == operation) {
        *result = a - b;
        return true;
    } else if('*' == operation) {
        *result = a * b;
        return true;
    } else if('/' == operation) {
        if(b == 0) {
            throw std::runtime_error("err: division by zero");
        }

        *result = a / b;
        return true;
    }

    return false;
}

bool try_evaluate_chop(long double a, long double b, char operation, long double* result, int chop) {

    a = get_chop(a, chop);
    b = get_chop(b, chop);

    if('+' == operation) {
        *result = get_chop(a + b, chop);
    } else if('-' == operation) {
        *result = get_chop(a - b, chop);
    } else if('*' == operation) {
        *result = get_chop(a * b, chop);
    } else if('/' == operation) {
        if(b == 0) {
            throw std::runtime_error("err: division by zero");
        }
    
        *result = get_chop(a / b, chop);
    } else {
        return false;
    }
    
    return true;
}

std::string remove_whitespace(std::string str) {
    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end()); 
    return str;
}

long double solve_expression(std::string expr) {
    
    const int chop = 8;

    expr = remove_whitespace(expr);

    std::stack<long double> nums;
    std::stack<char> oper;

    int i = 0;
    while(i < expr.length()) {
        // Digit 
        if(is_digit(expr[i])) {
            int num_len;
            nums.push(get_num_from_index(i, expr, &num_len));
            i += num_len - 1;
        } 

        // Operator (+, -, *, /, ^)
        else if(is_operator(expr[i])) {
            
            // check if '-' is a start of a negative number or is an operator 
            if(expr[i] == '-' && (i == 0 || is_operator(expr[i-1]) || expr[i-1] == '(') && is_digit(expr[i+1])) {
                int num_len;
                nums.push(get_num_from_index(i, expr, &num_len));
                i += num_len - 1;
            } else {
                while(!oper.empty() && get_precedence(oper.top()) >= get_precedence(expr[i])) {
                    long double operandB = nums.top();
                    nums.pop();
                    long double operandA = nums.top();
                    nums.pop();
                    char operation = oper.top();
                    oper.pop();

                    long double result;
                    if(try_evaluate(operandA, operandB, operation, &result)) {
                        nums.push(result);
                    }
                }
                
                oper.push(expr[i]);
            }
        } 

        // Left Parethesis
        else if(expr[i] == '(') {
            oper.push('(');
        } 

        // right parethesis
        else if(expr[i] == ')') {
            while(!oper.empty() && oper.top() != '(') {
                long double operandB = nums.top();
                nums.pop();
                long double operandA = nums.top();
                nums.pop();
                char operation = oper.top();
                oper.pop();

                long double result;
                if(try_evaluate(operandA, operandB, operation, &result)) {
                    nums.push(result);
                } else {
                    nums.push(0);
                }
            }
           
            if(!oper.empty() && oper.top() == '(') {
                oper.pop();
            }
        } 

        i++;
    }

    while(!oper.empty()) {
        long double operandB = nums.top();
        nums.pop();
        long double operandA = nums.top();
        nums.pop();
        char operation = oper.top();
        oper.pop();

        long double result;
        if(try_evaluate(operandA, operandB, operation, &result)) {
            nums.push(result);
        } else {
            nums.push(0);
        }
    }

    // last number in stack is the answer
        
    return nums.top();
}

int main() {
    std::string expressions[20] = {
        "45.67 * 2.3 + 18.92 / 4.4",
        "(100.5 - 45.22) * 1.5 / 0.5",
        "12.12 * 12.12 - 140.45",
        "88.88 / 2.2 + 9.99 * 3.0",
        "15.5 * (2.2 + 3.8) / 0.25",
        "0.75 * 0.25 + 1.25 / 0.5",
        "500.25 - (125.5 * 2.4)",
        "9.81 * 5.5 / 1.1 + 14.2",
        "102.4 / 3.2 * 1.5 - 8.8",
        "((12.5 + 7.5) * 3.2) / 0.8",
        "0.123 * 1000 - 45.67",
        "250.5 / 0.5 * 0.2 + 15.75",
        "14.4 * 1.2 + 13.3 / 0.7",
        "(99.99 / 3.33) * 1.1 - 2.2",
        "4.5 * 4.5 + 3.2 * 3.2",
        "1000.0 - (450.55 / 5.0)",
        "6.25 * 8.0 / 0.4 + 12.5",
        "15.75 + 14.25 - (5.5 * 2.0)",
        "0.5 * (120.4 / 4.0) + 18.9",
        "75.25 / 0.25 - 200.5"
    };

    long double answers[20] = {
        109.341,  
        165.84,  
        6.4444, 
        70.37, 
        372.0, 
        2.6875,
        199.05,
        63.25,
        39.2, 
        80.0, 
        77.33,
        115.95,  
        36.28,   
        30.8,    
        30.49,   
        909.89,  
        137.5,  
        19.0,   
        33.95,  
        100.5   
    };

    std::cout << std::fixed << std::setprecision(20);


    for(int i = 0; i < 20; i++) {
        std::string exp = expressions[i];
        long double answer = solve_expression(exp);
        
        std::string verdict;
        std::stringstream message;
    
        if(std::abs(answer - answers[i]) < 1e-1) {
            verdict = "pass... ";
        } else {
            verdict = "fail... ";
            message << " should be " << answers[i];
        }

        std::cout << verdict << "-> " << answer << message.str() << '\n';
    }

    return 0; 
}
