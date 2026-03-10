#include<bits/stdc++.h>

#define BIAS 1023            // value set for 64-bit 
#define EPSILON 1e-12        // threshold 
#define COMPUTE_PRECISION 12 // used for calculations
#define DISPLAY_PRECISION 8  // used for outputs

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
        if(whole % 2 > 0) {
            result.insert(0, 1, '1');
        } else {
            result.insert(0, 1, '0');
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

double get_chop(double n, int sdigit) {
    if(0.0 == n || !std::isfinite(n)) {
        return n;
    }

    double scale = std::pow(10.0, std::floor(std::log10(std::fabs(n))) - sdigit + 1);
    double scaled = n / scale;
    double chop = std::trunc(scaled);
    double chopped = chop * scale;

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
    return get_chop(ca + cb, chop);
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
    } else {
        throw std::runtime_error("Invalid operation");
    }

    return result;
}

double chop_evaluate(double a, double b, char oper, int sig) {
    a = get_chop(a, sig);
    b = get_chop(b, sig);

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
    } else {
        throw std::runtime_error("Invalid operation");
    }
    
    return get_chop(result, sig);
}

void remove_whitespace(std::string* str) {
    str->erase(std::remove_if(str->begin(), str->end(), ::isspace), str->end()); 
}

void solve_expression_helper(std::stack<double>* nums, std::stack<char>* opers) {
    if(nums->size() < 2) {
        throw std::runtime_error("Missing operand");
    }

    if(opers->empty() || opers->top() == '(') {
        throw std::runtime_error("Invalid expression");
    }

    double b = nums->top();
    nums->pop();
    double a = nums->top();
    nums->pop();
    char o = opers->top();
    opers->pop();

    nums->push(evaluate(a, b, o));
}

double solve_expression(std::string expr) {
    remove_whitespace(&expr);

    std::stack<double> nums;
    std::stack<char> opers;

    int i = 0;
    while(i < expr.length()) {
        // digit 
        if(is_digit(expr[i])) {
            int num_len;
            nums.push(get_num_from_index(i, expr, &num_len));
            i += num_len - 1;
        } 

        // operator (+, -, *, /, ^)
        else if(is_operator(expr[i])) {
            
            // check if '-' is a start of a negative number or is an operator 
            if(expr[i] == '-' && (i == 0 || is_operator(expr[i-1]) || expr[i-1] == '(') && is_digit(expr[i+1])) {
                int num_len;
                nums.push(get_num_from_index(i, expr, &num_len));
                i += num_len - 1;
            } else {
                while(!opers.empty() && get_precedence(opers.top()) >= get_precedence(expr[i])) {
                    solve_expression_helper(&nums, &opers); 
                }
                
                opers.push(expr[i]);
            }
        } 

        // left parenthesis
        else if(expr[i] == '(') {
            opers.push('(');
        } 

        // right parenthesis
        else if(expr[i] == ')') {
            while(!opers.empty() && opers.top() != '(') {
                solve_expression_helper(&nums, &opers); 
            }
           
            if(!opers.empty() && opers.top() == '(') {
                opers.pop();
            }
        } 

        i++;
    }

    while(!opers.empty()) {
        solve_expression_helper(&nums, &opers); 
    }

    return nums.top(); // last number in stack is the answer 
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

    double answers[20] = {
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
        double answer = solve_expression(exp);
        
        std::string verdict;
        std::stringstream message;
    
        if(std::abs(answer - answers[i]) < 1e-1) {
            verdict = "pass... ";
        } else {
            verdict = "fail... ";
            message << " should be " << answers[i];
        }

        std::cout << verdict << exp << " -> " << answer << message.str() << '\n';
    }

    return 0; 
}
