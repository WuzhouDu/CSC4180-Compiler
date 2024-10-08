# Copyright (c) 2024 Liu Yuxuan
# Email: yuxuanliu1@link.cuhk.edu.cn
#
# This code is licensed under MIT license (see LICENSE file for details)
#
# This code is for teaching purpose of course: CUHKSZ's CSC4180: Compiler Construction
# as Assignment 4: Oat v.1 Compiler Frontend to LLVM IR using llvmlite
#
# Copyright (c)
# Oat v.1 Language was designed by Prof.Steve Zdancewic when he was teaching CIS 341 at U Penn.
# 

import sys                          # for CLI argument parsing
import llvmlite.binding as llvm     # for llvmlite IR generation
import llvmlite.ir as ir            # for llvmlite IR generation
import pydot                        # for .dot file parsing
from enum import Enum               # for enum in python

DEBUG = False

class SymbolTable:
    """
    Symbol table is a stack of maps, and each map represents a scope

    The key of map is the lexeme (string, not unique) of an identifier, and the value is its type (DataType)

    The size of self.scopes and self.scope_ids should always be the same
    """
    def __init__(self):
        """
        Initialize the symbol table with no scope inside
        """
        self.id_counter = 0      # Maintain an increment counter for each newly pushed scope
        self.scopes = []         # map<str, DataType> from lexeme to data type in one scope
        self.scope_ids = []      # stores the ID for each scope

    def push_scope(self):
        """
        Push a new scope to symbol table

        Returns:
            - (int) the ID of the newly pushed scope
        """
        self.id_counter += 1
        self.scopes.append({})   # push a new table (mimics the behavior of stack)
        self.scope_ids.append(self.id_counter)
        return self.id_counter

    def pop_scope(self):
        """
        Pop a scope out of symbol table, usually called when the semantic analysis for one scope is finished
        """
        self.scopes.pop()    # pop out the last table (mimics the behavior of stack)
        self.scope_ids.pop()

    def unique_name(self, lexeme, scope_id):
        """
        Compute the unique name for an identifier in one certain scope

        Args:
            - lexeme(str)
            - scope_id(int)
        
        Returns:
            - str: the unique name of identifier used for IR codegen
        """
        return lexeme + "-" + str(scope_id)

    def insert(self, lexeme, type):
        """
        Insert a new symbol to the top scope of symbol table

        Args:
            - lexeme(str): lexeme of the symbol
            - type(DataType): type of the symbol
        
        Returns:
            - (str): the unique ID of the symbol
        """
        # check the size of scopes and scope_id
        if len(self.scopes) != len(self.scope_ids):
            raise ValueError("Mismatch size of symbol_table and id_table")
        scope_idx = len(self.scopes) - 1
        self.scopes[scope_idx][lexeme] = type
        return self.unique_name(lexeme, self.scope_ids[scope_idx])

    def lookup_local(self, lexeme):
        """
        Lookup a symbol in the top scope of symbol table only
        called when we want to declare a new local variable

        Args:
            - lexeme(str): lexeme of the symbol

        Returns:
            - (str, DataType): 2D tuple of unique_id and data type if the symbol is found
            - None if the symbol is not found
        """
        if len(self.scopes) != len(self.scope_ids):
            raise ValueError("Mismatch size of symbol_table and id_table")
        table_idx = len(self.scopes) - 1
        if lexeme in self.scopes[table_idx]:
            unique_name = self.unique_name(lexeme, self.scope_ids[table_idx])
            type = self.scopes[table_idx][lexeme]
            return unique_name, type
        else:
            return None

    def lookup_global(self, lexeme):
        """
        Lookup a symbol in all the scopes of symbol table (stack)
        called when we want to search a lexeme or declare a global variable

        Args:
            - lexeme(str): lexeme of the symbol

        Returns:
            - (str, DataType): 2D tuple of unique_id and data type if the symbol is found
            - None if the symbol is not found
        """
        if len(self.scopes) != len(self.scope_ids):
            raise ValueError("Mismatch size of symbol_table and id_table")
        for table_idx in range(len(self.scopes) - 1, -1, -1):
            if lexeme in self.scopes[table_idx]:
                unique_name = self.unique_name(lexeme, self.scope_ids[table_idx])
                type = self.scopes[table_idx][lexeme]
                return unique_name, type
        return None

# Global Symbol Table for Semantic Analysis
symbol_table = SymbolTable()

# Global Context of LLVM IR Code Generation
module = ir.Module()                    # Global LLVM IR Module
builder = ir.IRBuilder()                # Global LLVM IR Builder
ir_map = {}                             # Global Map from unique names to its LLVM IR item

class TreeNode:
    def __init__(self, index, lexeme):
        self.index = index              # [int] ID of the TreeNode, used for visualization 
        self.lexeme = lexeme            # [str] lexeme of the node (may have naming conflicts, and needs unique name in IR codegen)
        self.id = ""                    # [str] unique name of the node (used in IR codegen), filled in semantic analysis
        self.nodetype = NodeType.NONE   # [NodeType] type of node, used to determine which actions to do with the current node in semantic analysis or IR codegen
        self.datatype = DataType.NONE   # [DataType] data type of node, filled in semantic analysis and used in IR codegen
        self.children = []              # Array of childern TreeNodes

    def add_child(self, child_node):
        self.children.append(child_node)

def print_tree(node, level = 0):
    print("  " * level + '|' + node.lexeme.replace("\n","\\n") + ", " + node.nodetype.name)
    for child in node.children:
        print_tree(child, level + 1)

def visualize_tree(root_node, output_path):
    """
    Visulize TreeNode in Graphviz

    Args:
        - root_node(TreeNode)
        - output_path(str): the output path for png file 
    """
    # construct pydot Node from TreeNode with label
    def pydot_node(tree_node):
        node = pydot.Node(tree_node.index)
        label = tree_node.nodetype.name
        if len(tree_node.id) > 0:
            label += "\n" + tree_node.id.replace("\\n","\/n")
        elif len(tree_node.lexeme) > 0:
            label += "\n" + tree_node.lexeme.replace("\\n","\/n")
        label += "\ntype: " + tree_node.datatype.name
        node.set("label", label)
        return node
    # Recursively visualize node
    def visualize(node, graph):
        # Add Root Node Only
        if node.index == 0:
            graph.add_node(pydot_node(node))
        # Add Children Nodes and Edges
        for child in node.children:
            graph.add_node(pydot_node(child))
            graph.add_edge(pydot.Edge(node.index, child.index))
            visualize(child, graph)
    # Output visualization png graph
    graph = pydot.Dot(graph_type="graph")
    visualize(root_node, graph)
    graph.write_png(output_path)

def construct_tree_from_dot(dot_filepath):
    """
    Read .dot file, which records the AST from parser

    Args:
        - dot_filepath(str): path of the .dot file

    Return:
        - TreeNode: the root node of the AST
    """
    # Extract the first graph from the list (assuming there is only one graph in the file)
    graph = pydot.graph_from_dot_file(dot_filepath)[0]
    # Initialize Python TreeNode structure
    nodes = []
    # code_type_map = { member.value: member for member in NodeType }
    # Add nodes
    for node in graph.get_nodes():
        if len(node.get_attributes()) == 0: continue
        index = int(node.get_name()[4:])
        # print(node.get_attributes(), node.get_attributes()['label'], node.get_attributes()['lexeme'])
        label = node.get_attributes()["label"][1:-1]    # exlcude enclosing quotes
        lexeme = node.get_attributes()['lexeme'][1:-1]  # exclude enclosing quotes
        tree_node = TreeNode(index, lexeme)
        tree_node.lexeme = lexeme
        tree_node.nodetype = { member.value: member for member in NodeType }[label] if any(label == member.value for member in NodeType) else NodeType.NONE
        if DEBUG: print("Index: ", index, ", lexeme: ", lexeme, ", nodetype: ", tree_node.nodetype)
        nodes.append(tree_node)
    # Add Edges
    for edge in graph.get_edges():
        src_id = int(edge.get_source()[4:])
        dst_id = int(edge.get_destination()[4:])
        nodes[src_id].add_child(nodes[dst_id])
    # root node should always be the first node
    return nodes[0]

class NodeType(Enum):
    """
    Map lexeme of AST node to code type in IR generation

    The string value here is to convert the token class by bison to Python NodeType
    """
    PROGRAM = "<program>"
    GLOBAL_DECL = "<global_decl>"     # global variable declaration
    FUNC_DECL = "<function_decl>"       # function declaration
    VAR_DECLS = "<var_decls>"
    VAR_DECL = "<var_decl>"        # variable declaration
    ARGS = "<args>"
    ARG = "<arg>"
    REF = "<ref>"
    GLOBAL_EXPS = "<global_exps>"
    STMTS = "<stmts>"
    EXPS = "<exps>"
    FUNC_CALL = "<func call>"       # function call
    ARRAY_INDEX = "<array index>"    # array element retrieval by index
    IF_STMT = "IF"         # if-else statement (nested case included)
    ELSE_STMT = "ELSE"
    FOR_LOOP = "FOR"        # for loop statement (nested case included)
    WHILE_LOOP = "WHILE"      # while loop statement (nested case included)
    RETURN = "RETURN"          # return statement
    NEW = "NEW"         # new variable with/without initialization
    ASSIGN = "ASSIGN"         # assignment (lhs = rhs)
    EXP = "<exp>"     # expression (including a lot of binary operators)
    TVOID = "void"
    TINT = "int"
    TSTRING = "string"
    TBOOL = "bool"
    NULL = "NULL"
    TRUE = "TRUE"
    FALSE = "FALSE"
    STAR = "STAR"
    PLUS = "PLUS"
    MINUS = "MINUS"
    LSHIFT = "LSHIFT"
    RLSHIFT = "RLSHIFT"
    RASHIFT = "RASHIFT"
    LESS = "LESS"
    LESSEQ = "LESSEQ"
    GREAT = "GREAT"
    GREATEQ = "GREATEQ"
    EQ = "EQ"
    NEQ = "NEQ"
    LAND = "LAND"
    LOR = "LOR"
    BAND = "BAND"
    BOR = "BOR"
    NOT = "NOT"
    TILDE = "TILDE"
    INTLITERAL = "INTLITERAL"
    STRINGLITERAL = "STRINGLITERAL"
    ID = "ID"
    NONE = "unknown"         # unsupported

class DataType(Enum):
    INT = 1             # INT refers to Int32 type (32-bit Integer)
    BOOL = 2            # BOOL refers to Int1 type (1-bit Integer, 1 for True and 0 for False)
    STRING = 3          # STRING refers to an array of Int8 (8-bit integer for a single character)
    INT_ARRAY = 4       # Array of integers, no need to support unless for bonus
    BOOL_ARRAY = 5      # Array of booleans, no need to support unless for bonus
    STRING_ARRAY = 6    # Array of strings, no need to support unless for bonus
    VOID = 7            # Void, you can choose whether to support it or not
    NONE = 8            # Unknown type, used as initialized value for each TreeNode

def ir_type(data_type, array_size = 1):
    map = {
        DataType.INT: ir.IntType(32),       # integer is in 32-bit
        DataType.BOOL: ir.IntType(32),      # bool is also in 32-bit, 0 for false and 1 for True
        DataType.STRING: ir.ArrayType(ir.IntType(8), array_size + 1),   # extra \0 (null terminator)
        DataType.INT_ARRAY: ir.ArrayType(ir.IntType(32), array_size),
        DataType.BOOL_ARRAY: ir.ArrayType(ir.IntType(32), array_size),
        DataType.STRING_ARRAY: ir.ArrayType(ir.ArrayType(ir.IntType(8), 32), array_size),   # string max size = 32 for string array 
    }
    type = map.get(data_type)
    if type:
        return type
    else:
        raise ValueError("Unsupported data type: ", data_type)

def declare_runtime_functions():
    """
    Declare built-in functions for Oat v.1 Language
    """
    # int32_t* array_of_string (char *str)
    func_type = ir.FunctionType(
        ir.PointerType(ir.IntType(32)),     # return type
        [ir.PointerType(ir.IntType(8))])    # args type
    # map function unique name in global scope to the function body
    # the global scope should have scope_id = 1 
    func = ir.Function(module, func_type, name="array_of_string")
    ir_map[symbol_table.unique_name("array_of_string", 1)] = func
    # char* string_of_array (int32_t *arr)
    func_type = ir.FunctionType(
        ir.PointerType(ir.IntType(8)),      # return type
        [ir.PointerType(ir.IntType(32))])   # args type
    func = ir.Function(module, func_type, name="string_of_array")
    ir_map[symbol_table.unique_name("string_of_array", 1)] = func
    # int32_t length_of_string (char *str)
    func_type = ir.FunctionType(
        ir.IntType(32),                      # return type
        [ir.PointerType(ir.IntType(8))])    # args type
    func = ir.Function(module, func_type, name="length_of_string")
    ir_map[symbol_table.unique_name("length_of_string", 1)] = func
    # char* string_of_int(int32_t i)
    func_type = ir.FunctionType(
        ir.PointerType(ir.IntType(8)),      # return type
        [ir.IntType(32)])                   # args type
    func = ir.Function(module, func_type, name="string_of_int")
    ir_map[symbol_table.unique_name("string_of_int", 1)] = func
    # char* string_cat(char* l, char* r)
    func_type = ir.FunctionType(
        ir.PointerType(ir.IntType(8)),      # return tyoe
        [ir.PointerType(ir.IntType(8)), ir.PointerType(ir.IntType(8))]) # args type
    func = ir.Function(module, func_type, name="string_cat")
    ir_map[symbol_table.unique_name("string_cat", 1)] = func
    # void print_string (char* str)
    func_type = ir.FunctionType(
        ir.VoidType(),                      # return type
        [ir.PointerType(ir.IntType(8))])    # args type
    func = ir.Function(module, func_type, name="print_string")
    ir_map[symbol_table.unique_name("print_string", 1)] = func
    # void print_int (int32_t i)
    func_type = ir.FunctionType(
        ir.VoidType(),                      # return type
        [ir.IntType(32)])                   # args type
    func = ir.Function(module, func_type, name="print_int")
    ir_map[symbol_table.unique_name("print_int", 1)] = func
    # void print_bool (int32_t i)
    func_type = ir.FunctionType(
        ir.VoidType(),                      # return type
        [ir.IntType(32)])                   # args type
    func = ir.Function(module, func_type, name="print_bool")
    ir_map[symbol_table.unique_name("print_bool", 1)] = func

def codegen(node):
    """
    Recursively do LLVM IR generation

    Call corresponding handler function for each NodeType

    Different NodeTypes may be mapped to the same handelr function

    Args:
        node(TreeNode)
    """
    codegen_func_map = {
        NodeType.GLOBAL_DECL: codegen_handler_global_decl,
        NodeType.FUNC_DECL: codegen_handler_funcDec,
        NodeType.VAR_DECL: codegen_handler_varDec,
        NodeType.FUNC_CALL: codegen_handler_funCall,
        NodeType.RETURN: codegen_handler_ret,
        NodeType.ASSIGN: codegen_handler_assign,
        NodeType.IF_STMT: codegen_handler_if_stmt,
        NodeType.FOR_LOOP: codegen_handler_for_stmt,
        NodeType.WHILE_LOOP: codegen_handler_while_stmt,
        # TODO: add more mappings from NodeType to its handler function of IR generation
    }
    codegen_func = codegen_func_map.get(node.nodetype)
    if codegen_func:
        codegen_func(node)
    else:
        codegen_handler_default(node)

# Some sample handler functions for IR codegen
# TODO: implement more handler functions for various node types
def codegen_handler_default(node):
    for child in node.children:
        codegen(child)

def codegen_handler_global_decl(node):
    """
    Global variable declaration
    """
    var_name = node.children[0].lexeme
    variable = ir.GlobalVariable(module, typ=ir_type(node.children[0].datatype), name=var_name)
    variable.linkage = "private"
    variable.global_constant = True
    # initVal = codegen_handler_literal(node.children[1])
    initVal = node.children[1].lexeme
    variable.initializer = ir.Constant(ir_type(node.children[1].datatype), initVal)
    ir_map[node.children[0].id] = variable


def codegen_handler_literal(node):
    if (node.nodetype == NodeType.INTLITERAL):
        return ir.Constant(ir.IntType(32), int(node.lexeme))
    elif (node.nodetype == NodeType.STRINGLITERAL):
        # string_content = node.lexeme + '\0'
        # glbVar = ir.GlobalVariable(module, ir.ArrayType(ir.IntType(8), len(string_content)), name="string_literal_" + node.lexeme)
        # glbVar.linkage = "private"
        # glbVar.global_constant = True
        # glbVar.initializer = ir.Constant(ir.ArrayType(ir.IntType(8), len(string_content)), bytearray(string_content.encode('utf-8')))
        # zero = ir.Constant(ir.IntType(32), 0)
        # variable_pointer = builder.gep(glbVar, [zero, zero])
        # return variable_pointer
        return ir.Constant(ir.ArrayType(ir.IntType(8), len(node.lexeme) + 1), bytearray((node.lexeme + '\0').encode('utf-8')))
    elif (node.nodetype == NodeType.TRUE):
        return ir.Constant(ir.IntType(32), 1)
    elif (node.nodetype == NodeType.FALSE):
        return ir.Constant(ir.IntType(32), 0)
    else:
        raise ValueError("Unsupported literal type: ", node.nodetype)

def codegen_handler_funcDec(node):
    func_name = node.children[1].lexeme
    func_type = ir.FunctionType(ir_type(node.children[0].datatype), [])
    func = ir.Function(module, func_type, name=func_name)
    ir_map[node.children[1].id] = func
    block = func.append_basic_block("entry")
    global builder
    builder = ir.IRBuilder(block)
    for arg in node.children[2].children:
        arg_id = symbol_table.insert(arg.children[0].lexeme, arg.datatype)
        ir_map[arg.children[0].id] = builder.alloca(ir_type(arg.datatype), name=arg_id)
        builder.store(func.args[arg.index], ir_map[arg.children[0].id])
    codegen(node.children[3])


def codegen_handler_varDec(node):
    var_id = node.children[0].id
    if (node.children[0].datatype == DataType.STRING):
        ir_map[var_id] = builder.alloca(ir_type(DataType.STRING, len(node.children[1].lexeme)), name=var_id)
        builder.store(codegen_handler_literal(node.children[1]), ir_map[var_id])
    else: 
        ir_map[var_id] = builder.alloca(ir_type(node.children[0].datatype), name=var_id)
        initVal = codegen_handler_literal(node.children[1])
        builder.store(initVal, ir_map[var_id])

def codegen_handler_funCall(node):
    func_name = node.children[0].id
    func = ir_map[func_name]
    args = []
    for arg in node.children[1].children:
        args.append(codegen_handler_arg(arg))
    builder.call(func, args)

def codegen_handler_arg(node):
    if (node.nodetype == NodeType.ID):
        if (node.datatype == DataType.STRING):
            return builder.bitcast(ir_map[node.id], ir.PointerType(ir.IntType(8)))
        return builder.load(ir_map[node.id])
    else:
        if (node.datatype == DataType.STRING):
            string_literal_ptr = create_global_string(builder, node.lexeme + '\0', "string_literal_" + node.lexeme)
            return string_literal_ptr
        return codegen_handler_literal(node)

def codegen_handler_ret(node):
    builder.ret(codegen_handler_arg(node.children[0]))

def create_global_string(builder: ir.IRBuilder, s: str, name: str) -> ir.Instruction:
    type_i8_x_len = ir.types.ArrayType(ir.types.IntType(8), len(s))
    constant = ir.Constant(type_i8_x_len, bytearray(s.encode('utf-8')))
    variable = ir.GlobalVariable(builder.module, type_i8_x_len, name)
    variable.linkage = 'private'
    variable.global_constant = True
    variable.initializer = constant

    zero = ir.Constant(ir.IntType(32), 0)
    variable_pointer = builder.gep(variable, [zero, zero])
    return variable_pointer

def codegen_handler_assign(node):
    rhs = codegen_handler_operator(node.children[1])
    var_name = node.children[0].id
    builder.store(rhs, ir_map[var_name])

def codegen_handler_operator(node):
    if (node.nodetype == NodeType.PLUS):
        return builder.add(codegen_handler_arg(node.children[0]), codegen_handler_arg(node.children[1]))
    elif (node.nodetype == NodeType.MINUS):
        return builder.sub(codegen_handler_arg(node.children[0]), codegen_handler_arg(node.children[1]))

def codegen_handler_if_stmt(node):
    contion_node = node.children[0]
    condition = codegen_handler_condition(contion_node)
    if_block = builder.append_basic_block("if_block")
    if (len(node.children) == 2):
        merge_block = builder.append_basic_block("merge_block")
        builder.cbranch(condition, if_block, merge_block)
        builder.position_at_end(if_block)
        codegen(node.children[1])
        builder.branch(merge_block)
        builder.position_at_end(merge_block)
    else:
        else_block = builder.append_basic_block("else_block")
        merge_block = builder.append_basic_block("merge_block")
        builder.cbranch(condition, if_block, else_block)
        builder.position_at_end(if_block)
        codegen(node.children[1])
        builder.branch(merge_block)
        builder.position_at_end(else_block)
        codegen(node.children[2])
        builder.branch(merge_block)
        builder.position_at_end(merge_block)

def codegen_handler_condition(node):
    if (node.nodetype == NodeType.GREAT):
        return builder.icmp_signed(">", codegen_handler_arg(node.children[0]), codegen_handler_arg(node.children[1]))
    elif (node.nodetype == NodeType.GREATEQ):
        return builder.icmp_signed(">=", codegen_handler_arg(node.children[0]), codegen_handler_arg(node.children[1]))
    elif (node.nodetype == NodeType.LESS):
        return builder.icmp_signed("<", codegen_handler_arg(node.children[0]), codegen_handler_arg(node.children[1]))
    elif (node.nodetype == NodeType.LESSEQ):
        return builder.icmp_signed("<=", codegen_handler_arg(node.children[0]), codegen_handler_arg(node.children[1]))

def codegen_handler_for_stmt(node):
    initialization_node = node.children[0]
    codegen(initialization_node)

    loop_header = builder.append_basic_block("loop.header")
    loop_body = builder.append_basic_block("loop.body")
    loop_end = builder.append_basic_block("loop.end")

    builder.branch(loop_header)
    builder.position_at_end(loop_header)
    condition_node = node.children[1]
    condition = codegen_handler_condition(condition_node)
    builder.cbranch(condition, loop_body, loop_end)

    builder.position_at_end(loop_body)
    codegen(node.children[3])
    update_node = node.children[2]
    codegen(update_node)
    builder.branch(loop_header)

    builder.position_at_end(loop_end)

def codegen_handler_while_stmt(node):
    loop_header = builder.append_basic_block("loop.header")
    loop_body = builder.append_basic_block("loop.body")
    loop_end = builder.append_basic_block("loop.end")

    builder.branch(loop_header)
    builder.position_at_end(loop_header)
    condition_node = node.children[0]
    condition = codegen_handler_condition(condition_node)
    builder.cbranch(condition, loop_body, loop_end)

    builder.position_at_end(loop_body)
    codegen(node.children[1])
    builder.branch(loop_header)

    builder.position_at_end(loop_end)

def semantic_analysis(node):
    """
    Perform semantic analysis on the root_node of AST

    Args:
        node(TreeNode)

    Returns:
        (DataType): datatype of the node
    """
    handler_map = {
        NodeType.PROGRAM: semantic_handler_program,
        NodeType.ID: semantic_handler_id,
        NodeType.TINT: semantic_handler_int,
        NodeType.TBOOL: semantic_handler_bool,
        NodeType.TSTRING: semantic_handler_string,
        NodeType.INTLITERAL: semantic_handler_int,
        NodeType.STRINGLITERAL: semantic_handler_string,
        NodeType.TRUE: semantic_handler_bool,
        NodeType.FALSE: semantic_handler_bool,
        NodeType.GLOBAL_DECL: semantic_handler_globalDec,
        NodeType.FUNC_DECL: semantic_handler_funcDec,
        NodeType.VAR_DECL: semantic_handler_varDec,
        NodeType.FUNC_CALL: semantic_handler_funcCall,
        NodeType.ASSIGN: semantic_handler_assign,
        NodeType.PLUS: semantic_handler_plus,
        NodeType.RETURN: semantic_handler_return,
        NodeType.ARGS: semantic_handler_args,
        # NodeType.STMTS: semantic_handler_stmts,
        NodeType.IF_STMT: semantic_handler_ifStmt,
        NodeType.FOR_LOOP: semantic_handler_forLoop,
        NodeType.GREAT: semantic_handler_compare,
        NodeType.GREATEQ: semantic_handler_compare,
        NodeType.LESS: semantic_handler_compare,
        NodeType.LESSEQ: semantic_handler_compare,
        NodeType.WHILE_LOOP: semantic_handler_whileLoop,
        NodeType.MINUS: semantic_handler_minus,
        # TODO: add more mapping from NodeType to its corresponding handler functions here
    }
    handler = handler_map.get(node.nodetype)
    if handler:
        handler(node)
    else:
        default_handler(node)
    return node.datatype

def semantic_handler_program(node):
    symbol_table.push_scope()
    # insert built-in function names in global scope symbol table
    symbol_table.insert("array_of_string", DataType.INT_ARRAY)
    symbol_table.insert("string_of_array", DataType.STRING)
    symbol_table.insert("length_of_string", DataType.INT)
    symbol_table.insert("string_of_int", DataType.STRING)
    symbol_table.insert("string_cat", DataType.STRING)
    symbol_table.insert("print_string", DataType.VOID)
    symbol_table.insert("print_int", DataType.VOID)
    symbol_table.insert("print_bool", DataType.BOOL)
    # recursively do semantic analysis in left-to-right order for all children nodes
    for child in node.children:
        semantic_analysis(child)
    symbol_table.pop_scope()

# Some Sample handler functions
# TODO: define more hanlder functions for various node types
def semantic_handler_id(node):
    if symbol_table.lookup_global(node.lexeme) is None:
        raise ValueError("Variable not defined: ", node.lexeme)
    else:
        node.id, node.datatype = symbol_table.lookup_global(node.lexeme)

def semantic_handler_int(node):
    node.datatype = DataType.INT

def semantic_handler_bool(node):
    node.datatype = DataType.BOOL

def semantic_handler_string(node):
    node.datatype = DataType.STRING

def default_handler(node):
    for child in node.children:
        semantic_analysis(child)

def semantic_handler_globalDec(node):
    leftNode = node.children[0]
    rightNode = node.children[1]
    # print("global declaration: var: ", leftNode.lexeme, " val: ", rightNode.lexeme)
    if (leftNode.lexeme in symbol_table.scopes[0]):
        raise ValueError("Global variable already exist: ", leftNode.lexeme)
    else:
        semantic_analysis(rightNode)
        symbol_table.scopes[0][leftNode.lexeme] = rightNode.datatype
        leftNode.datatype = rightNode.datatype
        leftNode.id = leftNode.lexeme + "-1"

def semantic_handler_funcDec(node):
    typeNode = node.children[0]
    nameNode = node.children[1]
    # print("func declaration: id: ", nameNode.lexeme, " type: ", typeNode.lexeme)
    if (nameNode.lexeme in symbol_table.scopes[0]):
        raise ValueError("Function already exist: ", nameNode.lexeme)
    else:
        semantic_analysis(typeNode)
        symbol_table.scopes[0][nameNode.lexeme] = typeNode.datatype
        nameNode.datatype = typeNode.datatype
        nameNode.id = nameNode.lexeme + "-1"
        symbol_table.push_scope()
        for i in range(2, len(node.children)):
            semantic_analysis(node.children[i])
        symbol_table.pop_scope()

def semantic_handler_varDec(node):
    leftNode = node.children[0]
    rightNode = node.children[1]
    # print("variable declaration: var: ", leftNode.lexeme, " val: ", rightNode.lexeme)
    if (symbol_table.lookup_local(leftNode.lexeme) is not None):
        raise ValueError("Variable already exist: ", leftNode.lexeme)
    else:
        semantic_analysis(rightNode)
        symbol_table.insert(leftNode.lexeme, rightNode.datatype)
        leftNode.datatype = rightNode.datatype
        leftNode.id = leftNode.lexeme + "-" + str(symbol_table.scope_ids[-1])

def semantic_handler_funcCall(node):
    leftNode = node.children[0]
    rightNode = node.children[1]
    # print("function call: func: ", leftNode.lexeme, " args: ", rightNode.lexeme)
    if (leftNode.lexeme not in symbol_table.scopes[0]):
        raise ValueError("Function not defined: ", leftNode.lexeme)
    else:
        leftNode.datatype = symbol_table.scopes[0][leftNode.lexeme]
        leftNode.id = leftNode.lexeme + "-1"
        for i in range(len(rightNode.children)):
            semantic_analysis(rightNode.children[i])

def semantic_handler_assign(node):
    leftNode = node.children[0]
    rightNode = node.children[1]
    # print("assignment: lhs: ", leftNode.lexeme, " rhs: ", rightNode.lexeme)
    if (symbol_table.lookup_global(leftNode.lexeme) is None):
        raise ValueError("Variable not defined: ", leftNode.lexeme)
    else:
        semantic_analysis(rightNode)
        semantic_analysis(leftNode)

        if (leftNode.datatype != rightNode.datatype):
            # print("left: ", leftNode.lexeme, leftNode.id, leftNode.datatype)
            # print("right: ", rightNode.lexeme, rightNode.id, rightNode.datatype)
            raise ValueError("Variable assignment invalid")

def semantic_handler_plus(node):
    assert(len(node.children) == 2)
    leftNode = node.children[0]
    rightNode = node.children[1]
    # print("plus: lhs: ", leftNode.lexeme, " rhs: ", rightNode.lexeme)
    semantic_analysis(leftNode)
    semantic_analysis(rightNode)
    if ((leftNode.datatype != DataType.INT) or (leftNode.datatype != rightNode.datatype)):
        raise ValueError("Type mismatch in plus: ", leftNode.datatype, " and ", rightNode.datatype)
    else:
        node.datatype = leftNode.datatype

def semantic_handler_return(node):
    assert(len(node.children)) == 1
    # print("return: ", node.children[0].lexeme)
    semantic_analysis(node.children[0])
    node.datatype = node.children[0].datatype
           
def semantic_handler_args(node):
    if (len(node.children) == 0):
        node.datatype = DataType.VOID

def semantic_handler_ifStmt(node):
    compareNode = node.children[0]
    if (compareNode.nodetype == NodeType.GREAT or compareNode.nodetype == NodeType.GREATEQ or compareNode.nodetype == NodeType.LESS
        or compareNode.nodetype == NodeType.LESSEQ):
        semantic_analysis(node.children[0])
        assert(node.children[1].nodetype == NodeType.STMTS)
        symbol_table.push_scope()
        semantic_analysis(node.children[1])
        symbol_table.pop_scope()
        assert(len(node.children) == 3 and node.children[2].nodetype == NodeType.ELSE_STMT)
        symbol_table.push_scope()
        semantic_analysis(node.children[2])
        symbol_table.pop_scope()
    else:
        raise ValueError("If STMT has no condition")
    
def semantic_handler_forLoop(node):
    assert(len(node.children) == 4)
    initNode = node.children[0]
    compareNode = node.children[1]
    updateNode = node.children[2]
    assert(node.children[3].nodetype == NodeType.STMTS)
    if (initNode.nodetype == NodeType.VAR_DECLS):
        symbol_table.push_scope()
        semantic_analysis(initNode)
        semantic_analysis(compareNode)
        semantic_analysis(node.children[3])
        semantic_analysis(updateNode)
        symbol_table.pop_scope()
    else:
        raise ValueError("For loop has no initialization")

def semantic_handler_compare(node):
    assert(len(node.children) == 2)
    leftNode = node.children[0]
    rightNode = node.children[1]
    semantic_analysis(leftNode)
    semantic_analysis(rightNode)
    if ((leftNode.datatype != DataType.INT) or (leftNode.datatype != rightNode.datatype)):
        raise ValueError("Type mismatch in compare: ", leftNode.datatype, " and ", rightNode.datatype)
    else:
        node.datatype = DataType.BOOL

def semantic_handler_whileLoop(node):
    assert(len(node.children) == 2)
    compareNode = node.children[0]
    semantic_analysis(compareNode)
    symbol_table.push_scope()
    semantic_analysis(node.children[1])
    symbol_table.pop_scope()

def semantic_handler_minus(node):
    assert(len(node.children) == 2)
    leftNode = node.children[0]
    rightNode = node.children[1]
    # print("minus: lhs: ", leftNode.lexeme, " rhs: ", rightNode.lexeme)
    semantic_analysis(leftNode)
    semantic_analysis(rightNode)
    if ((leftNode.datatype != DataType.INT) or (leftNode.datatype != rightNode.datatype)):
        raise ValueError("Type mismatch in minus: ", leftNode.datatype, " and ", rightNode.datatype)
    else:
        node.datatype = leftNode.datatype

if len(sys.argv) == 3:
    # visualize AST before semantic analysis
    dot_path = sys.argv[1]
    ast_png_before_semantic_analysis = sys.argv[2]
    root_node = construct_tree_from_dot(dot_path)
    if DEBUG: print_tree(root_node)
    visualize_tree(root_node, ast_png_before_semantic_analysis)
elif len(sys.argv) == 4:
    # visualize AST after semantic analysis
    dot_path = sys.argv[1]
    ast_png_after_semantics_analysis = sys.argv[2]
    llvm_ir = sys.argv[3]
    root_node = construct_tree_from_dot(dot_path)
    semantic_analysis(root_node)
    visualize_tree(root_node, ast_png_after_semantics_analysis)
    # Uncomment the following when you are trying the do IR generation
    # init llvm
    llvm.initialize()
    llvm.initialize_native_target()
    llvm.initialize_native_asmprinter()
    declare_runtime_functions()
    codegen(root_node)
    # print LLVM IR)
    with open(llvm_ir, 'w') as f:
        f.write(str(module))
    # print(str(module))
else:
    raise SyntaxError("Usage: python3 a4.py <.dot> <.png before>\nUsage: python3 ./a4.py <.dot> <.png after> <.ll>")
