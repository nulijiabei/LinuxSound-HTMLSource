if (! this.sh_languages) {
  this.sh_languages = {};
}
sh_languages['m4'] = [
  [
    {
      'next': 1,
      'regex': /dnl/g,
      'state': 1,
      'style': 'sh_keyword'
    },
    {
      'next': 7,
      'regex': /#+/g,
      'style': 'sh_comment'
    },
    {
      'regex': /\b(?:import)\b/g,
      'style': 'sh_preproc'
    },
    {
      'regex': /\b[+-]?(?:(?:0x[A-Fa-f0-9]+)|(?:(?:[\d]*\.)?[\d]+(?:[eE][+-]?[\d]+)?))u?(?:(?:int(?:8|16|32|64))|L)?\b/g,
      'style': 'sh_number'
    },
    {
      'regex': /\\"/g,
      'style': 'sh_normal'
    },
    {
      'regex': /\\'/g,
      'style': 'sh_normal'
    },
    {
      'next': 8,
      'regex': /"/g,
      'style': 'sh_string'
    },
    {
      'next': 9,
      'regex': /'/g,
      'style': 'sh_string'
    },
    {
      'regex': /function[ \t]+(?:[A-Za-z]|_)[A-Za-z0-9_]*[ \t]*(?:\(\))?/g,
      'style': 'sh_function'
    },
    {
      'regex': /(?:[A-Za-z]|_)[A-Za-z0-9_]*[ \t]*\(\)/g,
      'style': 'sh_function'
    },
    {
      'regex': /(?:[A-Za-z]*[-\/]+[A-Za-z]+)+/g,
      'style': 'sh_normal'
    },
    {
      'regex': /\b(?:alias|bg|bind|break|builtin|caller|case|command|compgen|complete|continue|declare|dirs|disown|do|done|elif|else|enable|esac|eval|exec|exit|export|false|fc|fg|fi|for|getopts|hash|help|history|if|in|jobs|let|local|logout|popd|printf|pushd|read|readonly|return|select|set|shift|shopt|source|suspend|test|then|times|trap|true|type|typeset|umask|unalias|unset|until|wait|while)\b/g,
      'style': 'sh_keyword'
    },
    {
      'regex': /(?:[A-Za-z]|_)[A-Za-z0-9_]*(?==)/g,
      'style': 'sh_variable'
    },
    {
      'regex': /\$\{(?:[^ \t]+)\}/g,
      'style': 'sh_variable'
    },
    {
      'regex': /\$\((?:[^ \t]+)\)/g,
      'style': 'sh_variable'
    },
    {
      'regex': /\$(?:[A-Za-z]|_)[A-Za-z0-9_]*/g,
      'style': 'sh_variable'
    },
    {
      'regex': /\$(?:[^ \t]{1})/g,
      'style': 'sh_variable'
    },
    {
      'regex': /~|!|%|\^|\*|\(|\)|\+|=|\[|\]|\\|:|;|,|\.|\/|\?|&|<|>|\||(?:##){2}|%%/g,
      'style': 'sh_symbol'
    },
    {
      'next': 10,
      'regex': /#/g,
      'style': 'sh_comment'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'regex': /(?:<?)[A-Za-z0-9_\.\/\-_]+@[A-Za-z0-9_\.\/\-_]+(?:>?)/g,
      'style': 'sh_url'
    },
    {
      'regex': /(?:<?)[A-Za-z0-9_]+:\/\/[A-Za-z0-9_\.\/\-_]+(?:>?)/g,
      'style': 'sh_url'
    },
    {
      'next': 2,
      'regex': /<!DOCTYPE/g,
      'state': 1,
      'style': 'sh_preproc'
    },
    {
      'next': 4,
      'regex': /<!--/g,
      'style': 'sh_comment'
    },
    {
      'regex': /<(?:\/)?[A-Za-z][A-Za-z0-9]*(?:\/)?>/g,
      'style': 'sh_keyword'
    },
    {
      'next': 5,
      'regex': /<(?:\/)?[A-Za-z][A-Za-z0-9]*/g,
      'state': 1,
      'style': 'sh_keyword'
    },
    {
      'regex': /&(?:[A-Za-z0-9]+);/g,
      'style': 'sh_preproc'
    },
    {
      'regex': /@[A-Za-z]+/g,
      'style': 'sh_type'
    },
    {
      'regex': /[^ \t]+/g,
      'style': 'sh_comment'
    },
    {
      'regex': /[ \t]+/g,
      'style': 'sh_comment'
    }
  ],
  [
    {
      'exit': true,
      'regex': />/g,
      'style': 'sh_preproc'
    },
    {
      'next': 3,
      'regex': /"/g,
      'style': 'sh_string'
    }
  ],
  [
    {
      'regex': /\\(?:\\|")/g
    },
    {
      'exit': true,
      'regex': /"/g,
      'style': 'sh_string'
    }
  ],
  [
    {
      'exit': true,
      'regex': /-->/g,
      'style': 'sh_comment'
    },
    {
      'next': 4,
      'regex': /<!--/g,
      'style': 'sh_comment'
    }
  ],
  [
    {
      'exit': true,
      'regex': /(?:\/)?>/g,
      'style': 'sh_keyword'
    },
    {
      'regex': /[^=" \t>]+/g,
      'style': 'sh_type'
    },
    {
      'regex': /=/g,
      'style': 'sh_symbol'
    },
    {
      'next': 6,
      'regex': /"/g,
      'style': 'sh_string'
    }
  ],
  [
    {
      'regex': /\\(?:\\|")/g
    },
    {
      'exit': true,
      'regex': /"/g,
      'style': 'sh_string'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    }
  ],
  [
    {
      'exit': true,
      'regex': /"/g,
      'style': 'sh_string'
    },
    {
      'regex': /\\./g,
      'style': 'sh_specialchar'
    }
  ],
  [
    {
      'exit': true,
      'regex': /'/g,
      'style': 'sh_string'
    },
    {
      'regex': /\\./g,
      'style': 'sh_specialchar'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    }
  ]
];
