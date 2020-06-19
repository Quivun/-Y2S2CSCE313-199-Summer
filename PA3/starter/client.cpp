class Solution {
public:
    // Splits an input string given a dividing character(s). O(n).
    vector<string> splitter (string inp, string div){
        vector<string> ret;
        if (div.length() == 0){
            return ret;
        }
        string cur = "";
        for (int q =0; q <= inp.length() - div.length(); q++){
            if (inp.substr(q,div.length()) == div){
                if (cur.length() > 0){
                    ret.push_back(cur);
                }
                cur = "";
            } else {
                cur += inp.substr(q,1);
            }
        }
        if (cur.length() > 0){
            ret.push_back(cur);
        }
        return ret;
    }
    // Locates a character given an input string. O(n).
    bool stringIn(string inp, string item){
        for (int q = 0; q < inp.length(); q++){
            if (item == inp.substr(q,item.length())){
                return true;
            }
        }
        return false;
    }
    int sToInt(string inp){
        int ret = 0;
        int dec = 1;
        for (int q = inp.length()-1; q > -1; q--){
            char c = inp[q];
            if (!(c >= 48 && c <= 57)){
                return -1;
            } else {
                ret += dec*(((int)c)-48);
            }
            dec *= 10;
        }
        return ret;
    }
    // Given a string, return true or false if the string is an acceptable input for ipv4. 
    bool isIP4(string inp){
        // Check witihn range of 0,255
        if ( inp == "0" ){
            return true;
        }
        
        /*
        if (atoi(inp) > 1 && atoi(inp) < 256 && noLeadIP4(inp)){
            return true;
        }
        // Atoi doesn't work
        */
        if (sToInt(inp) >= 1 && sToInt(inp) <= 255 && noLeadIP4(inp)){
            return true;
        }
        return false;
    }
    // Given a string, return true if there's no leading zeroes, false otherwise.
    bool noLeadIP4(string inp){
        int cnt = 0;
        for (int q = 0; q <inp.length();q ++){
            if( inp.substr(q,1) == "0" ){
                cnt++;
            } else {
                q = inp.length();
            }
        }
        if (cnt > 0){
            return false; // There's leading zeroes
        }
        return true;
    }
    // Given a string, return true or false if the string is an acceptable input for ipv6.
    bool isIP6(string inp){
        if (inp.size() > 4 || inp.size() < 1){
            return false;
        }
        bool c1,c2,c3;
        for (int q = 0; q < inp.size(); q++){
            c1 = false;
            c2 = false;
            c3 = false;
            char c = inp[q];
            if ( c >= '0' && c <= '9'){
                c1 = true;
            }
            if ( c >= 'A' && c <= 'F'){
                c2 = true;
            }
            if ( c >= 'a' && c <= 'f'){
                c3 = true;
            }
            if (!(c1||c2||c3)){
                return false;
            }
        }
        return true;
    }
    string validIPAddress(string IP) {
        // "IPv4";
        if (stringIn(IP,".")){
            vector<string> vStr1 = splitter(IP,".");
            for (int q = 0; q < vStr1.size(); q++){
                if (!isIP4(vStr1[q]) || vStr1.size() != 4){
                    return "Neither";
                }
            }
            return "IPv4";
        } 
        // "IPv6" Goes from 0-9 and a-f inclusive;
        if (stringIn(IP,":")){
            vector<string> vStr1 = splitter(IP,":");
            for (int q = 0; q < vStr1.size(); q++){
                if (!isIP6(vStr1[q]) || vStr1.size() != 8 ){
                    return "Neither";
                }
            }
            return "IPv6";
        }
        // "Neither";
        return "Neither";
    }
};