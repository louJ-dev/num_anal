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
    int sign = ('-' == binary[i++]) * -1;
    
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
    int e = binary_to_decimal(binary.substr(1, 11));
    double fraction = 1.0;
    
    long double mult = 1.0;
    for(char b : binary.substr(12, 51)) {
        fraction += (b - '0') / mult;
        mult *= 2.0;
    }

    return (double)sign * (double)(std::pow(2, e - 1023)) * fraction;
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
