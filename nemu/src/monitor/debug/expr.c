#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {  //start from 256 to avoid ascii
  TK_NOTYPE = 256, 
  TK_HEX,
  TK_DEC,
  TK_REG, 
  TK_EQ, 
  TK_NEQ, 
  TK_AND, 
  TK_OR,
  TK_NEG,      //negative number
  TK_POI,       //use for get the content of pointer

  /* TODO: Add more token types */

};
static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"0x[0-9A-Fa-f][0-9A-Fa-f]*", TK_HEX},
  {"0|[1-9][0-9]*", TK_DEC},
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)", TK_REG},

  {"\\+", '+'},         
  {"-", '-'},          
  {"\\*", '*'},
  {"\\/", '/'},

  {"\\(", '('},
  {"\\)", ')'},
  
  {"==", TK_EQ},         
  {"!=", TK_NEQ},

  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"!", '!'},

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(rules[i].token_type == TK_NOTYPE) //�give up space
            break;
        if(substr_len>31)  //avoid overflow
            assert(0);
        strncpy(tokens[nr_token].str, substr_start, substr_len);
        tokens[nr_token].type = rules[i].token_type;
             
        nr_token = nr_token + 1;

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p,int q){
    if((tokens[p].str[0]=='(') && (tokens[q].str[0]==')')){
        int count = 0;
        for(int i=p;i<q;i++) //
        {
            if(tokens[i].str[0] == '(')
                count = count + 1;
            if(tokens[i].str[0] == ')')
                count = count - 1;
            if(count == 0)
            {
                //printf("Leftmost and rightmost are not matched\n");
                return false;
            }
        }
        count = count -1; 
        if(count !=0) 
        {
            printf("Bad parentheses\n");
            assert(0);
        }
        return true;
    }
    else
    {
        // printf("The whole expr was not surrounded\n");
        return false;
    }
}


uint32_t eval(int p,int q){
    if(p>q){   //3+缺省为3+0 --1缺省为0--1
        // printf("Bad expression\n");
        return 0;
        // assert(0);
    }
    else if(p==q){
        uint32_t res;
        if(tokens[p].type == TK_HEX) sscanf(tokens[p].str,"%x",&res);
        else if(tokens[p].type == TK_DEC) sscanf(tokens[p].str,"%d",&res);
        else if(tokens[p].type == TK_REG){
            char tmp[3] = {tokens[p].str[1],tokens[p].str[2],tokens[p].str[3]};
            for(int i=0;i<8;i++)
                if(!strcmp(tmp,regsl[i])){return cpu.gpr[i]._32;}
            for(int i=0;i<8;i++)
                if(!strcmp(tmp,regsw[i])){return cpu.gpr[i]._16;}
            for(int i=0;i<8;i++) 
                if(!strcmp(tmp,regsb[i])){return cpu.gpr[i%4]._8[i/4];}
	    char teip[3]="eip";
	    if(strcmp(tmp,teip))return cpu.eip;
        }
        else assert(0);
        return res;
    }
    else if(check_parentheses(p,q) == true){
        return eval(p+1,q-1);
    }
    else{
        int op=0;
        int op_type=0;
        bool left = false;//
        int curr_prev = 100;//
        for(int i=p;i<=q;i++){  
            if(tokens[i].str[0]==')')
            {
                left = false;
                continue;
            }
            if(left)
                continue;
            if(tokens[i].str[0]=='(')
            {
                left = true;
                continue;
            }
            switch(tokens[i].type){
                case TK_OR:if(curr_prev>1){curr_prev=1;op=i;op_type=TK_OR;continue;}
                case TK_AND:if(curr_prev>2){curr_prev=2;op=i;op_type=TK_AND;continue;}
                case TK_NEQ:if(curr_prev>3){curr_prev=3;op=i;op_type=TK_NEQ;continue;}
                case TK_EQ:if(curr_prev>3){curr_prev=3;op=i;op_type=TK_EQ;continue;}
                case '<':if(curr_prev>4){curr_prev=4;op=i;op_type='<';continue;}
                case '>':if(curr_prev>4){curr_prev=4;op=i;op_type='>';continue;}
                case '+':if(curr_prev>6){curr_prev=6;op=i;op_type='+';continue;}
                case '-':if(curr_prev>6){curr_prev=6;op=i;op_type='-';continue;}
                case '*':if(curr_prev>7){curr_prev=7;op=i;op_type='*';continue;}
                case '/':if(curr_prev>7){curr_prev=7;op=i;op_type='/';continue;}
                case '!':if(curr_prev>8){curr_prev=8;op=i;op_type='!';continue;}
                case TK_NEG:if(curr_prev>9){curr_prev=9;op=i;op_type=TK_NEG;continue;}
                case TK_POI:if(curr_prev>9){curr_prev=9;op=i;op_type=TK_POI;continue;}
                default:continue;
            }
        }

        uint32_t val1 = eval(p,op-1);
        uint32_t val2 = eval(op+1,q);
        switch(op_type){
            case TK_OR:return val1||val2;
            case TK_AND:return val1&&val2;
            case TK_NEQ:return val1!=val2;
            case TK_EQ:return val1==val2;
            case '<':return val1<val2;
            case '>':return val1>val2;
            case '+':return val1+val2;
            case '-':return val1-val2;
            case '*':return val1*val2;
            case '/':return val1/val2;
            case '!':return !val2;
            case TK_NEG:return -1*val2; 
            case TK_POI:return vaddr_read(val2,4);
            default:assert(0);
        }
    }
}
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if(nr_token!=1) 
    for(int i=0;i<nr_token;i++) 
        if(tokens[i].type == '-' &&(i==0||tokens[i-1].type == '('||tokens[i-1].type == TK_NEG
                                                                 ||tokens[i-1].type == '-'
                                                                 ||tokens[i-1].type == '+'
                                                                 ||tokens[i-1].type == '*'
                                                                 ||tokens[i-1].type == '/'))
            tokens[i].type = TK_NEG;
 // if(nr_token!=1)
      for(int i=0;i<nr_token;i++)
          if(tokens[i].type == '*' &&(i==0||(tokens[i-1].type!=TK_DEC && tokens[i-1].type!=TK_HEX && tokens[i-1].type!=')')))
              tokens[i].type = TK_POI;

  //*success = true;
  /* TODO: Insert codes to evaluate the expression. */
  // TODO();  
  //
  printf("RESULT=%d\n",eval(0, nr_token-1));

  return eval(0, nr_token-1);
}
