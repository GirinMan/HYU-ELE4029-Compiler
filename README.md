# HYU_ELE4029_Compiler
2022 Compiler Design - Prof. Yongjun Park (Dept. of Computer Science)

# 개요
- Tiny의 컴파일러를 수정하여 가상의 언어인 C-Minus의 컴파일러를 개발합니다.
- 각 과제는 별도의 프로젝트가 아닌, 기존 개발 내용을 기반으로 내용이 이어집니다.
- 자세한 내용은 레포트 pdf 파일을 참조하세요.

# 1_Scanner
C-minus로 작성된 프로그램의 lexical analysis를 위해 적절한 reserved keyword, symbol, 그리고 token 등을 정의한 다음 DFA를 활용하여 입력받은 문자열을 token 단위로 분리할 수 있게 하는 scanner를 만드는 과제입니다.

# 2_Parser
C-minus로 작성된 프로그램의 lexical analysis를 위해 적절한 reserved keyword, symbol, 그리고 token 등을 정의한 다음 Yacc/Bision을 이용해 정의하는 grammar를 이용해 parsing을 진행하는 프로그램을 개발합니다.

# 3_Semantic
C-minus로 작성된 프로그램의 semantic analysis를 위해 적절한 reserved keyword, symbol, 그리고 token 등을 정의한 다음 Yacc/Bision을 이용해 정의하는 grammar를 이용해 parsing을 진행한 결과로 AST가 생성되면, 이를 바탕으로 symbol table을 생성하고 type checking 등 semantic analysis를 진행합니다.
