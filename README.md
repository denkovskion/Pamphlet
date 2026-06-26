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
- `-d`, `--detailed` Enables detailed analysis.
- `-v`, `--verbose` Enables verbose logging.

## Example

> Sam Loyd, The Sunny South 1885

### Input

```
5Q2/5B1k/6r1/6p1/6N1/6K1/8/8 w - - dm 2;
```

### Output (default)

```
Qf8-a8 [#2]
```

### Output with `--detailed`

```
1.Qf8-a8 Rg6-f6 2.Qa8-g8#
1...Rg6-e6 2.Qa8-g8#
1...Rg6-d6 2.Qa8-g8#
1...Rg6-c6 2.Qa8-g8#
1...Rg6-b6 2.Qa8-g8#
1...Rg6-a6 2.Qa8-g8#
1...Rg6-g7 2.Qa8-h1#
1...Rg6-g8 2.Qa8xg8#
1...Rg6-h6 2.Qa8-g8#
1...Kh7-g7 2.Qa8-g8#
```

## Author

Ivan Denkovski is the author of Pamphlet.
