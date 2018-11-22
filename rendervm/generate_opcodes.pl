#!/usr/bin/env perl

use strict;
use warnings;

use constant SIZEOF_ADDR    => 2;
use constant SIZEOF_STACK   => 1;
use constant SIZEOF_UINT8   => 1;
use constant SIZEOF_UINT16  => 2;
use constant SIZEOF_UINT32  => 4;
use constant SIZEOF_FLOAT   => 4;

my @CORE = qw(
    HALT
    YIELD
    RESET
    CALL
    RETURN
    JUMP
);

my @TYPES = qw(
    UINT8
    UINT16
    UINT32
    FLOAT
    VEC2
    VEC3
    VEC4
    MAT2
    MAT3
    MAT4
);

my @STACK_OPS = qw(
    POP
    DUP
    SWAP
    JUMPEM
);

my @MEMORY_OPS = qw(
    STORE
    LOAD
);

my @MATH_OPS = qw(
    ADD
    SUB
    MUL
    EQ
);

my %BASE_ARGLEN = (
    HALT => {
        desc   => "Halts the VM and but does not reset",
        arglen => 0,
        stacks => [],
    },
    YIELD => {
        desc   => "Stops the machine but leaves the state in-tact. When the machine is started again it will resume from the next instruction.",
        arglen => 0,
        stacks => [],
    },
    RESET => {
        desc   => "Resets the internal registers of the VM and stops the machine.",
        arglen => 0,
        stacks => [],
    },
    CALL => {
        desc   => "Adds the next instruction address to the ctrl stack and jumps to the specified address.",
        arglen => SIZEOF_ADDR,
        stacks => [qw(ctrl.+)],
    },
    RETURN => {
        desc   => "Pops the last address off the ctrl stack and jumps to this address.",
        arglen => 0,
        stacks => [qw(ctrl.-)],
    },
    JUMP => {
        desc   => "Jumps to the specified address.",
        arglen => SIZEOF_ADDR,
        stacks => [],
    },
    POP => {
        desc   => "Removes N items from the top of the [STACK] stack, disgarding the values.",
        arglen => SIZEOF_STACK,
        stacks => [qw(STACK.-)],
    },
    DUP => {
        desc   => "Copies N items from the top of the [STACK] stack and pushes them again.",
        arglen => SIZEOF_STACK,
        stacks => [qw(STACK.+)],
    },
    SWAP => {
        desc   => "Swaps the top two items on the [STACK] stack.",
        arglen => 0,
        stacks => [qw(STACK.-- STACK.++)],
    },
    ROT => {
        desc   => "",
        arglen => SIZEOF_ADDR,
        stacks => [],
    },
    JUMPEM => {
        desc   => "Jump to an address if the [STACK] stack is empty",
        arglen => SIZEOF_ADDR,
        stacks => [],
    },
    STORE => {
        desc   => "Takes an address from the uint16 stack, removes a value from the [STACK] stack and stores that value in memory.",
        arglen => 0,
        stacks => [qw(STACK.-)],
    },
    LOAD => {
        desc   => "Takes an address from the uint16 stack and loads a value from memory, putting the value on the [STACK] stack.",
        arglen => 0,
        stacks => [qw(STACK.+)],
    },
    ADD => {
        desc   => "Takes two from the [STACK] stack and adds them, returning the result to the [STACK] stack.",
        arglen => 0,
        stacks => [qw(STACK.-- STACK.+)],
    },
    SUB => {
        desc   => "Takes two from the stack and subtracts them, returning the result to the stack.",
        arglen => 0,
        stacks => [qw(STACK.-- STACK.+)],
    },
    MUL => {
        desc   => "Takes two from the stack and multiplies them, returning the result to the stack.",
        arglen => 0,
        stacks => [qw(STACK.-- STACK.+)],
    },
    EQ => {
        desc   => "Takes two from the stack, performs an equality check and places a 1 or 0 on the uint8 stack.",
        arglen => 0,
        stacks => [qw(STACK.-- uint8.+)],
    },
);

my %CUSTOM = (
    UINT8 => {
        JUMPZ => {
            desc   => "Jump to the specified address if the [STACK] value is zero.",
            arglen => SIZEOF_ADDR,
            stacks => [qw(STACK.-)],
        },
        JUMPNZ => {
            desc   => "Jump to the specified address if the [STACK] value not zero.",
            arglen => SIZEOF_ADDR,
            stacks => [qw(STACK.-)],
        },
        PUSH => {
            desc   => "Push a constant value onto the [STACK] stack.",
            arglen => SIZEOF_UINT8,
            stacks => [qw(STACK.+)],
        },
    },
    UINT16 => {
        JUMPZ => {
            desc   => "Jump to the specified address if the [STACK] value is zero.",
            arglen => SIZEOF_ADDR,
            stacks => [qw(STACK.-)],
        },
        JUMPNZ => {
            desc   => "Jump to the specified address if the [STACK] value not zero.",
            arglen => SIZEOF_ADDR,
            stacks => [qw(STACK.-)],
        },
        PUSH => {
            desc   => "Push a constant value onto the [STACK] stack.",
            arglen => SIZEOF_UINT16,
            stacks => [qw(STACK.+)],
        },
    },
    UINT32 => {
        JUMPZ => {
            desc   => "Jump to the specified address if the [STACK] value is zero.",
            arglen => SIZEOF_ADDR,
            stacks => [qw(STACK.-)],
        },
        JUMPNZ => {
            desc   => "Jump to the specified address if the [STACK] value not zero.",
            arglen => SIZEOF_ADDR,
            stacks => [qw(STACK.-)],
        },
        PUSH => {
            desc   => "Push a constant value onto the [STACK] stack.",
            arglen => SIZEOF_UINT32,
            stacks => [qw(STACK.+)],
        },
    },
    FLOAT => {
        JUMPZ => {
            desc   => "Jump to the specified address if the [STACK] value is zero.",
            arglen => SIZEOF_ADDR,
            stacks => [qw(STACK.-)],
        },
        JUMPNZ => {
            desc   => "Jump to the specified address if the [STACK] value not zero.",
            arglen => SIZEOF_ADDR,
            stacks => [qw(STACK.-)],
        },
        PUSH => {
            desc   => "Push a constant value onto the [STACK] stack.",
            arglen => SIZEOF_FLOAT,
            stacks => [qw(STACK.+)],
        },
    },
    VEC2 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 2 float values and pushes them onto the float stack.",
            arglen => 0,
            stacks => [qw(STACK.- float.++)],
        },
        IMPLODE => {
            desc   => "Takes 2 values from the float stack, creates a new [STACK] item and pushes it.",
            arglen => 0,
            stacks => [qw(float.-- STACK.+)],
        },
        MULMAT2 => {
            desc   => "Multiply the value by the mat2 matrix on top of the mat2 stack.",
            arglen => 0,
            stacks => [qw(STACK.- mat2.- STACK.+)],
        },
        MULMAT3 => {
            desc   => "Multiply the value by the mat3 matrix on top of the mat3 stack.",
            arglen => 0,
            stacks => [qw(STACK.- mat3.- STACK.+)],
        },
        MULMAT4 => {
            desc   => "Multiply the value by the mat4 matrix on top of the mat4 stack.",
            arglen => 0,
            stacks => [qw(STACK.- mat4.- STACK.+)],
        },
    },
    VEC3 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 3 float values and pushes them onto the float stack.",
            arglen => 0,
            stacks => [qw(STACK.- float.+++)],
        },
        IMPLODE => {
            desc   => "Takes 3 values from the float stack, creates a new [STACK] item and pushes it.",
            arglen => 0,
            stacks => [qw(float.--- STACK.+)],
        },
        MULMAT3 => {
            desc   => "Multiply the value by the mat3 matrix on top of the mat3 stack.",
            arglen => 0,
            stacks => [qw(STACK.- mat3.- STACK.+)],
        },
        MULMAT4 => {
            desc   => "Multiply the value by the mat4 matrix on top of the mat4 stack.",
            arglen => 0,
            stacks => [qw(STACK.- mat4.- STACK.+)],
        },
    },
    VEC4 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 4 float values and pushes them onto the float stack.",
            arglen => 0,
            stacks => [qw(STACK.- float.++++)],
        },
        IMPLODE => {
            desc   => "Takes 4 values from the float stack, creates a new [STACK] item and pushes it.",
            arglen => 0,
            stacks => [qw(float.---- STACK.+)],
        },
        MULMAT4 => {
            desc   => "Multiply the value by the mat4 matrix on top of the mat4 stack.",
            arglen => 0,
            stacks => [qw(STACK.- mat4.- STACK.+)],
        },
    },
    MAT2 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 4 float values and pushes them onto the float stack.",
            arglen => 0,
            stacks => [qw(STACK.- float.++++)],
        },
        IDENT => {
            desc   => "Set the identity matrix.",
            arglen => 0,
            stacks => [qw(STACK.- STACK.+)],
        },
        IMPLODE => {
            desc   => "Takes 4 values from the float stack, creates a new [STACK] item and pushes it.",
            arglen => 0,
            stacks => [qw(float.---- STACK.+)],
        },
        ROTATE => {
            desc   => "Add a rotation component to the matrix.",
            arglen => 0,
            stacks => [qw(STACK.- float.- STACK.+)],
        },
        SCALE => {
            desc   => "Add a scale component to the matrix.",
            arglen => 0,
            stacks => [qw(STACK.- vec2.- STACK.+)],
        },
        TRANSP => {
            desc   => "Transpose the matrix.",
            arglen => 0,
            stacks => [qw(STACK.- STACK.+)],
        },
    },
    MAT3 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 9 float values and pushes them onto the float stack.",
            arglen => 0,
            stacks => [qw(STACK.- float.+++++++++)],
        },
        IDENT => {
            desc   => "Set the identity matrix.",
            arglen => 0,
            stacks => [qw(STACK.- STACK.+)],
        },
        IMPLODE => {
            desc   => "Takes 9 values from the float stack, creates a new [STACK] item and pushes it.",
            arglen => 0,
            stacks => [qw(float.--------- STACK.+)],
        },
        ROTATE => {
            desc   => "Add a rotation component to the matrix.",
            arglen => 0,
            stacks => [qw(STACK.- float.- STACK.+)],
        },
        SCALE => {
            desc   => "Add a scale component to the matrix.",
            arglen => 0,
            stacks => [qw(STACK.- vec2.- STACK.+)],
        },
        TRANSL => {
            desc   => "Add a translation component to the matrix.",
            arglen => 0,
            stacks => [qw(STACK.- vec2.- STACK.+)],
        },
        TRANSP => {
            desc   => "Transpose the matrix.",
            arglen => 0,
            stacks => [qw(STACK.- STACK.+)],
        },
    },
    MAT4 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 16 float values and pushes them onto the float stack.",
            arglen => 0,
            stacks => [qw(STACK.- float.++++++++++++++++)],
        },
        IDENT => {
            desc   => "Set the identity matrix.",
            arglen => 0,
            stacks => [qw(STACK.- STACK.+)],
        },
        IMPLODE => {
            desc   => "Takes 16 values from the float stack, creates a new [STACK] item and pushes it.",
            arglen => 0,
            stacks => [qw(float.---------------- STACK.+)],
        },
        ROTATEX => {
            desc   => "Add a rotation component to the matrix around the X axis.",
            arglen => 0,
            stacks => [qw(STACK.- float.- STACK.+)],
        },
        ROTATEY => {
            desc   => "Add a rotation component to the matrix around the Y axis.",
            arglen => 0,
            stacks => [qw(STACK.- float.- STACK.+)],
        },
        ROTATEZ => {
            desc   => "Add a rotation component to the matrix around the Z axis.",
            arglen => 0,
            stacks => [qw(STACK.- float.- STACK.+)],
        },
        SCALE => {
            desc   => "Add a scale component to the matrix.",
            arglen => 0,
            stacks => [qw(STACK.- vec3.- STACK.+)],
        },
        TRANSL => {
            desc   => "Add a translation component to the matrix.",
            arglen => 0,
            stacks => [qw(STACK.- vec3.- STACK.+)],
        },
        TRANSP => {
            desc   => "Transpose the matrix.",
            arglen => 0,
            stacks => [qw(STACK.- STACK.+)],
        },
    },
);

my @all_ops;
my $idx = 0;
for my $opcode (@CORE) {
    my $arglen = $BASE_ARGLEN{$opcode}->{arglen};
    push @all_ops, {
        opcode => $opcode,
        code   => $idx++,
        desc   => $BASE_ARGLEN{$opcode}->{desc},
        arglen => $arglen,
        stack  => 'ctrl',
        stacks => $BASE_ARGLEN{$opcode}->{stacks},
    }
}
for my $type (@TYPES) {
    for my $op (@STACK_OPS) {
        my $opcode = join('_', $type, $op);
        my $arglen = $BASE_ARGLEN{$op}->{arglen};
        push @all_ops, {
            opcode => $opcode,
            code   => $idx++,
            desc   => $BASE_ARGLEN{$op}->{desc},
            arglen => $arglen,
            stack  => lc($type),
            stacks => $BASE_ARGLEN{$op}->{stacks},
        }
    }
    for my $op (@MEMORY_OPS) {
        my $opcode = join('_', $type, $op);
        my $arglen = $BASE_ARGLEN{$op}->{arglen};
        push @all_ops, {
            opcode => $opcode,
            code   => $idx++,
            desc   => $BASE_ARGLEN{$op}->{desc},
            arglen => $arglen,
            stack  => lc($type),
            stacks => $BASE_ARGLEN{$op}->{stacks},
        }
    }
    for my $op (@MATH_OPS) {
        my $opcode = join('_', $type, $op);
        my $arglen = $BASE_ARGLEN{$op}->{arglen};
        push @all_ops, {
            opcode => $opcode,
            code   => $idx++,
            desc   => $BASE_ARGLEN{$op}->{desc},
            arglen => $arglen,
            stack  => lc($type),
            stacks => $BASE_ARGLEN{$op}->{stacks},
        }
    }
    for my $op (sort keys %{$CUSTOM{$type}}) {
        my $arglen = $CUSTOM{$type}->{$op}->{arglen};
        my $opcode = join('_', $type, $op);
        push @all_ops, {
            opcode => $opcode,
            code   => $idx++,
            desc   => $CUSTOM{$type}->{$op}->{desc},
            arglen => $arglen,
            stack  => lc($type),
            stacks => $CUSTOM{$type}->{$op}->{stacks},
        }
    }
}

printf("my %%INS_ARG_LENGTH = (\n");
for my $op (@all_ops) {
    printf("    %-20s=> [0x%02x, [0x%02x]],\n", $op->{opcode}, $op->{code}, $op->{arglen});
}
printf(");\n");

for my $op (@all_ops) {
    printf("## %s (0x%02x):\n\n", lc($op->{opcode}), $op->{code});
    my $stack = $op->{stack};
    my $desc = $op->{desc};
    if ($desc) {
        $desc =~ s/\[STACK\]/$stack/g;
        printf("%s\n\n", $desc);
    }
    my @stacks = @{$op->{stacks}};
    for my $st (@stacks) {
        if ($st =~ /^STACK/) {
            $st =~ s/^STACK/$stack/;
        }
        my $desc = "";
        if ($st =~ /^([0-9a-z]+)\.([\-\+]+)/) {
            my ($stack, $str) = ($1, $2);
            my @dirs = split(//, $str);
            my $removesadds = $dirs[0] eq '-' ? 'removes' : 'adds';
            my $fromto = $dirs[0] eq '-' ? 'from' : 'to';
            my $nr = scalar(@dirs);
            my $pl = ($nr > 1) ? 's' : '';
            $desc = "$removesadds $nr item$pl $fromto the $stack stack";
        }
        print "  $desc\n";
    }
    print "\n";
}

