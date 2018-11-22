#!/usr/bin/env perl

use strict;
use warnings;

=for explanation

This file is to make my like a bit easier when adding new opcodes to the VM.
When you run this script it will output a Perl hash that starts: "my
%INS_ARG_LENGTH = (" - this should be copy-pasted into the top of the
"rendervmasm" Perl script. Once that is done:

To generate the enum definition for rendervm.h:

  ./rendervmasm --cenum

To generate an empty switch statement for rendervm.c:

  ./rendervmasm --cswitch

To generate the string array for debugging for rendervm.c:

  ./rendervmasm --copcode2str

To generate a list of test functions and the main calling function for test.c:

  ./rendervmasm --ctests

=cut

use constant TYPE_ADDR  => 'uint16';
use constant TYPE_STACK => 'uint8';

my @CORE = qw(
    HALT
    YIELD
    RESET
    CALL
    RETURN
    JUMP
);

my @STACKS = qw(
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
        type   => [],
        stacks => [],
    },
    YIELD => {
        desc   => "Stops the machine but leaves the state in-tact. When the machine is started again it will resume from the next instruction.",
        type   => [],
        stacks => [],
    },
    RESET => {
        desc   => "Resets the internal registers of the VM and stops the machine.",
        type   => [],
        stacks => [],
    },
    CALL => {
        desc   => "Adds the next instruction address to the ctrl stack and jumps to the specified address.",
        type   => [TYPE_ADDR],
        stacks => [qw(ctrl.+)],
    },
    RETURN => {
        desc   => "Pops the last address off the ctrl stack and jumps to this address.",
        type   => [],
        stacks => [qw(ctrl.-)],
    },
    JUMP => {
        desc   => "Jumps to the specified address.",
        type   => [TYPE_ADDR],
        stacks => [],
    },
    POP => {
        desc   => "Removes N items from the top of the [STACK] stack, disgarding the values.",
        type   => [TYPE_STACK],
        stacks => [qw(STACK.-)],
    },
    DUP => {
        desc   => "Copies N items from the top of the [STACK] stack and pushes them again.",
        type   => [TYPE_STACK],
        stacks => [qw(STACK.+)],
    },
    SWAP => {
        desc   => "Swaps the top two items on the [STACK] stack.",
        type   => [],
        stacks => [qw(STACK.-- STACK.++)],
    },
    ROT => {
        desc   => "",
        type   => [TYPE_STACK],
        stacks => [],
    },
    JUMPEM => {
        desc   => "Jump to an address if the [STACK] stack is empty",
        type   => [TYPE_ADDR],
        stacks => [],
    },
    STORE => {
        desc   => "Takes an address from the uint16 stack, removes a value from the [STACK] stack and stores that value in memory.",
        type   => [],
        stacks => [qw(STACK.-)],
    },
    LOAD => {
        desc   => "Takes an address from the uint16 stack and loads a value from memory, putting the value on the [STACK] stack.",
        type   => [],
        stacks => [qw(STACK.+)],
    },
    ADD => {
        desc   => "Takes two from the [STACK] stack and adds them, returning the result to the [STACK] stack.",
        type   => [],
        stacks => [qw(STACK.-- STACK.+)],
    },
    SUB => {
        desc   => "Takes two from the stack and subtracts them, returning the result to the stack.",
        type   => [],
        stacks => [qw(STACK.-- STACK.+)],
    },
    MUL => {
        desc   => "Takes two from the stack and multiplies them, returning the result to the stack.",
        type   => [],
        stacks => [qw(STACK.-- STACK.+)],
    },
    EQ => {
        desc   => "Takes two from the stack, performs an equality check and places a 1 or 0 on the uint8 stack.",
        type   => [],
        stacks => [qw(STACK.-- uint8.+)],
    },
);

my %CUSTOM = (
    UINT8 => {
        JUMPZ => {
            desc   => "Jump to the specified address if the [STACK] value is zero.",
            type   => [TYPE_ADDR],
            stacks => [qw(STACK.-)],
        },
        JUMPNZ => {
            desc   => "Jump to the specified address if the [STACK] value not zero.",
            type   => [TYPE_ADDR],
            stacks => [qw(STACK.-)],
        },
        PUSH => {
            desc   => "Push a constant value onto the [STACK] stack.",
            type   => ['uint8'],
            stacks => [qw(STACK.+)],
        },
    },
    UINT16 => {
        JUMPZ => {
            desc   => "Jump to the specified address if the [STACK] value is zero.",
            type   => [TYPE_ADDR],
            stacks => [qw(STACK.-)],
        },
        JUMPNZ => {
            desc   => "Jump to the specified address if the [STACK] value not zero.",
            type   => [TYPE_ADDR],
            stacks => [qw(STACK.-)],
        },
        PUSH => {
            desc   => "Push a constant value onto the [STACK] stack.",
            type   => ['uint16'],
            stacks => [qw(STACK.+)],
        },
        MOVE_UINT8 => {
            desc   => "Move values from the uint8 stack to the [STACK] stack.",
            type   => [TYPE_STACK],
            stacks => [qw(uint8.- STACK.+)],
        },
    },
    UINT32 => {
        JUMPZ => {
            desc   => "Jump to the specified address if the [STACK] value is zero.",
            type   => [TYPE_ADDR],
            stacks => [qw(STACK.-)],
        },
        JUMPNZ => {
            desc   => "Jump to the specified address if the [STACK] value not zero.",
            type   => [TYPE_ADDR],
            stacks => [qw(STACK.-)],
        },
        PUSH => {
            desc   => "Push a constant value onto the [STACK] stack.",
            type   => ['uint32'],
            stacks => [qw(STACK.+)],
        },
        MOVE_UINT8 => {
            desc   => "Move values from the uint8 stack to the [STACK] stack.",
            type   => [TYPE_STACK],
            stacks => [qw(uint8.- STACK.+)],
        },
    },
    FLOAT => {
        JUMPZ => {
            desc   => "Jump to the specified address if the [STACK] value is zero.",
            type   => [TYPE_ADDR],
            stacks => [qw(STACK.-)],
        },
        JUMPNZ => {
            desc   => "Jump to the specified address if the [STACK] value not zero.",
            type   => [TYPE_ADDR],
            stacks => [qw(STACK.-)],
        },
        PUSH => {
            desc   => "Push a constant value onto the [STACK] stack.",
            type   => ['float'],
            stacks => [qw(STACK.+)],
        },
    },
    VEC2 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 2 float values and pushes them onto the float stack.",
            type   => [],
            stacks => [qw(STACK.- float.++)],
        },
        IMPLODE => {
            desc   => "Takes 2 values from the float stack, creates a new [STACK] item and pushes it.",
            type   => [],
            stacks => [qw(float.-- STACK.+)],
        },
        MULMAT2 => {
            desc   => "Multiply the value by the mat2 matrix on top of the mat2 stack.",
            type   => [],
            stacks => [qw(STACK.- mat2.- STACK.+)],
        },
        MULMAT3 => {
            desc   => "Multiply the value by the mat3 matrix on top of the mat3 stack.",
            type   => [],
            stacks => [qw(STACK.- mat3.- STACK.+)],
        },
        MULMAT4 => {
            desc   => "Multiply the value by the mat4 matrix on top of the mat4 stack.",
            type   => [],
            stacks => [qw(STACK.- mat4.- STACK.+)],
        },
    },
    VEC3 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 3 float values and pushes them onto the float stack.",
            type   => [],
            stacks => [qw(STACK.- float.+++)],
        },
        IMPLODE => {
            desc   => "Takes 3 values from the float stack, creates a new [STACK] item and pushes it.",
            type   => [],
            stacks => [qw(float.--- STACK.+)],
        },
        MULMAT3 => {
            desc   => "Multiply the value by the mat3 matrix on top of the mat3 stack.",
            type   => [],
            stacks => [qw(STACK.- mat3.- STACK.+)],
        },
        MULMAT4 => {
            desc   => "Multiply the value by the mat4 matrix on top of the mat4 stack.",
            type   => [],
            stacks => [qw(STACK.- mat4.- STACK.+)],
        },
    },
    VEC4 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 4 float values and pushes them onto the float stack.",
            type   => [],
            stacks => [qw(STACK.- float.++++)],
        },
        IMPLODE => {
            desc   => "Takes 4 values from the float stack, creates a new [STACK] item and pushes it.",
            type   => [],
            stacks => [qw(float.---- STACK.+)],
        },
        MULMAT4 => {
            desc   => "Multiply the value by the mat4 matrix on top of the mat4 stack.",
            type   => [],
            stacks => [qw(STACK.- mat4.- STACK.+)],
        },
    },
    MAT2 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 4 float values and pushes them onto the float stack.",
            type   => [],
            stacks => [qw(STACK.- float.++++)],
        },
        IDENT => {
            desc   => "Set the identity matrix.",
            type   => [],
            stacks => [qw(STACK.- STACK.+)],
        },
        IMPLODE => {
            desc   => "Takes 4 values from the float stack, creates a new [STACK] item and pushes it.",
            type   => [],
            stacks => [qw(float.---- STACK.+)],
        },
        ROTATE => {
            desc   => "Add a rotation component to the matrix.",
            type   => [],
            stacks => [qw(STACK.- float.- STACK.+)],
        },
        SCALE => {
            desc   => "Add a scale component to the matrix.",
            type   => [],
            stacks => [qw(STACK.- vec2.- STACK.+)],
        },
        TRANSP => {
            desc   => "Transpose the matrix.",
            type   => [],
            stacks => [qw(STACK.- STACK.+)],
        },
    },
    MAT3 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 9 float values and pushes them onto the float stack.",
            type   => [],
            stacks => [qw(STACK.- float.+++++++++)],
        },
        IDENT => {
            desc   => "Set the identity matrix.",
            type   => [],
            stacks => [qw(STACK.- STACK.+)],
        },
        IMPLODE => {
            desc   => "Takes 9 values from the float stack, creates a new [STACK] item and pushes it.",
            type   => [],
            stacks => [qw(float.--------- STACK.+)],
        },
        ROTATE => {
            desc   => "Add a rotation component to the matrix.",
            type   => [],
            stacks => [qw(STACK.- float.- STACK.+)],
        },
        SCALE => {
            desc   => "Add a scale component to the matrix.",
            type   => [],
            stacks => [qw(STACK.- vec2.- STACK.+)],
        },
        TRANSL => {
            desc   => "Add a translation component to the matrix.",
            type   => [],
            stacks => [qw(STACK.- vec2.- STACK.+)],
        },
        TRANSP => {
            desc   => "Transpose the matrix.",
            type   => [],
            stacks => [qw(STACK.- STACK.+)],
        },
    },
    MAT4 => {
        EXPLODE => {
            desc   => "Takes a value form the [STACK] stack, unpacks the 16 float values and pushes them onto the float stack.",
            type   => [],
            stacks => [qw(STACK.- float.++++++++++++++++)],
        },
        IDENT => {
            desc   => "Set the identity matrix.",
            type   => [],
            stacks => [qw(STACK.- STACK.+)],
        },
        IMPLODE => {
            desc   => "Takes 16 values from the float stack, creates a new [STACK] item and pushes it.",
            type   => [],
            stacks => [qw(float.---------------- STACK.+)],
        },
        ROTATEX => {
            desc   => "Add a rotation component to the matrix around the X axis.",
            type   => [],
            stacks => [qw(STACK.- float.- STACK.+)],
        },
        ROTATEY => {
            desc   => "Add a rotation component to the matrix around the Y axis.",
            type   => [],
            stacks => [qw(STACK.- float.- STACK.+)],
        },
        ROTATEZ => {
            desc   => "Add a rotation component to the matrix around the Z axis.",
            type   => [],
            stacks => [qw(STACK.- float.- STACK.+)],
        },
        SCALE => {
            desc   => "Add a scale component to the matrix.",
            type   => [],
            stacks => [qw(STACK.- vec3.- STACK.+)],
        },
        TRANSL => {
            desc   => "Add a translation component to the matrix.",
            type   => [],
            stacks => [qw(STACK.- vec3.- STACK.+)],
        },
        TRANSP => {
            desc   => "Transpose the matrix.",
            type   => [],
            stacks => [qw(STACK.- STACK.+)],
        },
    },
);

my @all_ops;
my $idx = 0;
for my $opcode (@CORE) {
    my $type = $BASE_ARGLEN{$opcode}->{type};
    push @all_ops, {
        opcode => $opcode,
        code   => $idx++,
        desc   => $BASE_ARGLEN{$opcode}->{desc},
        type   => $type,
        stack  => 'ctrl',
        stacks => $BASE_ARGLEN{$opcode}->{stacks},
    }
}
for my $stack (@STACKS) {
    for my $op (@STACK_OPS) {
        my $opcode = join('_', $stack, $op);
        my $type = $BASE_ARGLEN{$op}->{type};
        push @all_ops, {
            opcode => $opcode,
            code   => $idx++,
            desc   => $BASE_ARGLEN{$op}->{desc},
            type   => $type,
            stack  => lc($stack),
            stacks => $BASE_ARGLEN{$op}->{stacks},
        }
    }
    for my $op (@MEMORY_OPS) {
        my $opcode = join('_', $stack, $op);
        my $type = $BASE_ARGLEN{$op}->{type};
        push @all_ops, {
            opcode => $opcode,
            code   => $idx++,
            desc   => $BASE_ARGLEN{$op}->{desc},
            type   => $type,
            stack  => lc($stack),
            stacks => $BASE_ARGLEN{$op}->{stacks},
        }
    }
    for my $op (@MATH_OPS) {
        my $opcode = join('_', $stack, $op);
        my $type = $BASE_ARGLEN{$op}->{type};
        push @all_ops, {
            opcode => $opcode,
            code   => $idx++,
            desc   => $BASE_ARGLEN{$op}->{desc},
            type   => $type,
            stack  => lc($stack),
            stacks => $BASE_ARGLEN{$op}->{stacks},
        }
    }
    for my $op (sort keys %{$CUSTOM{$stack}}) {
        my $type = $CUSTOM{$stack}->{$op}->{type};
        my $opcode = join('_', $stack, $op);
        push @all_ops, {
            opcode => $opcode,
            code   => $idx++,
            desc   => $CUSTOM{$stack}->{$op}->{desc},
            type   => $type,
            stack  => lc($stack),
            stacks => $CUSTOM{$stack}->{$op}->{stacks},
        }
    }
}

printf("my %%INS_ARG_LENGTH = (\n");
for my $op (@all_ops) {
    printf(
        "    %-20s=> [0x%02x, [%s]],\n",
        $op->{opcode},
        $op->{code},
        join(',', map {"'$_'"} @{$op->{type}})
    );
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

