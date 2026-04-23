# Pamphlet

Pamphlet is a mate search chess program.

## Usage

```
Pamphlet [OPTIONS]
```

Pamphlet reads problems
as [Extended Position Description](https://www.chessprogramming.org/Extended_Position_Description)
records (with one operation: `dm` for direct mate or `acd` for perft) from standard input until
end-of-file, then solves them and writes solutions to standard output.

## Options

- `-h`, `--help` Shows help and exits.
- `-V`, `--version` Shows version and exits.

## Example

> Sam Loyd, The Sunny South 1885

### Input

```
5Q2/5B1k/6r1/6p1/6N1/6K1/8/8 w - - dm 2;
```

### Output

```
Qf8-a8 [#2]
```

## Author

Ivan Denkovski is the author of Pamphlet.
