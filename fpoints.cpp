#include<bits/stdc++.h>

std::string decimal_to_binary(double n) {
    if(0 == n) {
        return "0";
    }

    int sign = 1;

    if(n < 0) sign = -1;

    n = std::fabs(n);

    int whole = (int)n;
    double decimal = n - whole;

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

std::string decimal_to_ieee(double n) {
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
    
    long double mult = 2.0;
    for(char b : binary.substr(12, 51)) {
        fraction += (b - '0') * (1.0 / mult);
        // std::cout << std::to_string(fraction) << '\n';
        mult *= 2.0;
    }

    return sign * std::pow(2, (e - 1023)) * fraction;
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

bool IsDigit(char c) {
    if(c >= '0' && c <= '9') {
        return true;
    }

    return false;
}

bool IsOperator(char c) {
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

int GetPrecedence(char operation) {
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
//      stops until non-numeric char
// returns: zero when no number is found
int GetNumFromIndex(int i, string s, int *length) { 
    int sign = 1;
    int len = 0;
    
    if(s[i] == '-') {
        sign = -1;
        len++;
        i++;
    }

    int num = 0;
    while(i < s.length() && IsDigit(s[i])) {
        num = (num * 10) + (s[i] - '0');
        len++;
        i++;   
    }
    
    *length = len;

    return num * sign;
}

bool TryEvaluate(int operandA, int operandB, char operation, int *result) {
    if(operation == '+') {
        *result = operandA + operandB;
        return true;
    } else if(operation == '-') {
        *result = operandA - operandB;
        return true;
    } else if(operation == '*') {
        *result = operandA * operandB;
        return true;
    } else if(operation == '/') {
        if(operandB == 0) {
            *result = NULL;
            return false;
        }

        *result = operandA / operandB;
        return true;
    }

    return false;
}

string RemoveWhitespace(string str) {
    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end()); 
    return str;
}

int SolveExpression(string expr) {
    expr = RemoveWhitespace(expr);

    stack<int> nums;
    stack<char> oper;

    int i = 0;
    while(i < expr.length()) {
        // Digit 
        if(IsDigit(expr[i])) {
            int num_len;
            nums.push(GetNumFromIndex(i, expr, &num_len));
            i += num_len - 1;
        } 

        // Operator (+, -, *, /, ^)
        else if(IsOperator(expr[i])) {
            
            // check if '-' is a start of a negative number or is an operator 
            if(expr[i] == '-' && (i == 0 || IsOperator(expr[i-1]) || expr[i-1] == '(') && IsDigit(expr[i+1])) {
                int num_len;
                nums.push(GetNumFromIndex(i, expr, &num_len));
                i += num_len - 1;
            } else {
                while(!oper.empty() && GetPrecedence(oper.top()) >= GetPrecedence(expr[i])) {
                    int operandB = nums.top();
                    nums.pop();
                    int operandA = nums.top();
                    nums.pop();
                    char operation = oper.top();
                    oper.pop();

                    int result;
                    if(TryEvaluate(operandA, operandB, operation, &result)) {
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
                int operandB = nums.top();
                nums.pop();
                int operandA = nums.top();
                nums.pop();
                char operation = oper.top();
                oper.pop();

                int result;
                if(TryEvaluate(operandA, operandB, operation, &result)) {
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
        int operandB = nums.top();
        nums.pop();
        int operandA = nums.top();
        nums.pop();
        char operation = oper.top();
        oper.pop();

        int result;
        if(TryEvaluate(operandA, operandB, operation, &result)) {
            nums.push(result);
        } else {
            nums.push(0);
        }
    }

    // last number in stack is the answer
    return nums.top();
}

int main() {
   
    std::cout << decimal_to_ieee(1) << '\n';
    std::cout << decimal_to_ieee(0) << '\n';
    std::cout << decimal_to_ieee(-1) << '\n';
    std::cout << decimal_to_ieee(1.5) << '\n';
    std::cout << decimal_to_ieee(-1.5) << '\n';

    std::cout << ieee_to_decimal("0011111111110000000000000000000000000000000000000000000000000000") << '\n';
    std::cout << ieee_to_decimal("0000000000000000000000000000000000000000000000000000000000000000") << '\n';
    std::cout << ieee_to_decimal("1011111111110000000000000000000000000000000000000000000000000000") << '\n';
    std::cout << ieee_to_decimal("0011111111111000000000000000000000000000000000000000000000000000") << '\n';
    std::cout << ieee_to_decimal("1011111111111000000000000000000000000000000000000000000000000000") << '\n';

    return 0; 
}
