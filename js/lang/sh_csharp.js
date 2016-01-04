if (! this.sh_languages) {
  this.sh_languages = {};
}
sh_languages['csharp'] = [
  [
    {
      'regex': /\b(?:using)\b/g,
      'style': 'sh_preproc'
    },
    {
      'regex': /\b[+-]?(?:(?:0x[A-Fa-f0-9]+)|(?:(?:[\d]*\.)?[\d]+(?:[eE][+-]?[\d]+)?))(?:[FfDdMmUulL]+)?\b/g,
      'style': 'sh_number'
    },
    {
      'next': 1,
      'regex': /\/\/\//g,
      'style': 'sh_comment'
    },
    {
      'next': 7,
      'regex': /\/\//g,
      'style': 'sh_comment'
    },
    {
      'next': 8,
      'regex': /\/\*\*/g,
      'style': 'sh_comment'
    },
    {
      'next': 14,
      'regex': /\/\*/g,
      'style': 'sh_comment'
    },
    {
      'next': 15,
      'regex': /^[ \t]*#(?:[ \t]*include)/g,
      'state': 1,
      'style': 'sh_preproc'
    },
    {
      'regex': /^[ \t]*#(?:[ \t]*[A-Za-z0-9_]*)/g,
      'style': 'sh_preproc'
    },
    {
      'regex': /\b[+-]?(?:(?:0x[A-Fa-f0-9]+)|(?:(?:[\d]*\.)?[\d]+(?:[eE][+-]?[\d]+)?))u?(?:(?:int(?:8|16|32|64))|L)?\b/g,
      'style': 'sh_number'
    },
    {
      'next': 32,
      'regex': /"/g,
      'style': 'sh_string'
    },
    {
      'next': 33,
      'regex': /'/g,
      'style': 'sh_string'
    },
    {
      'regex': /(\b(?:class|struct|typename))([ \t]+)([A-Za-z0-9]+)/g,
      'style': ['sh_keyword', 'sh_normal', 'sh_type']
    },
    {
      'regex': /\b(?:abstract|event|new|struct |as|explicit|null|switch|base|extern|this|false|operator|throw|break|finally|out|true|fixed|override|try|case|params|typeof|catch|for|private|foreach|protected|checked|goto|public|unchecked|class|if|readonly|unsafe|const|implicit|ref|continue|in|return|virtual|default|interface|sealed|volatile|delegate|internal|do|is|sizeof|while|lock|stackalloc|else|static|enum|namespace|get|partial|set|value|where|yield)\b/g,
      'style': 'sh_keyword'
    },
    {
      'regex': /\b(?:bool|byte|sbyte|char|decimal|double|float|int|uint|long|ulong|object|short|ushort|string|void)\b/g,
      'style': 'sh_type'
    },
    {
      'regex': /~|!|%|\^|\*|\(|\)|-|\+|=|\[|\]|\\|:|;|,|\.|\/|\?|&|<|>|\|/g,
      'style': 'sh_symbol'
    },
    {
      'regex': /\{|\}/g,
      'style': 'sh_cbracket'
    },
    {
      'regex': /(?:[A-Za-z]|_)[A-Za-z0-9_]*[ \t]*(?=\()/g,
      'style': 'sh_function'
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
      'regex': /(?:TODO|FIXME)(?:[:]?)/g,
      'style': 'sh_todo'
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
      'regex': /\*\//g,
      'style': 'sh_comment'
    },
    {
      'next': 8,
      'regex': /\/\*\*/g,
      'style': 'sh_comment'
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
      'next': 9,
      'regex': /<!DOCTYPE/g,
      'state': 1,
      'style': 'sh_preproc'
    },
    {
      'next': 11,
      'regex': /<!--/g,
      'style': 'sh_comment'
    },
    {
      'regex': /<(?:\/)?[A-Za-z][A-Za-z0-9]*(?:\/)?>/g,
      'style': 'sh_keyword'
    },
    {
      'next': 12,
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
      'regex': /(?:TODO|FIXME)(?:[:]?)/g,
      'style': 'sh_todo'
    }
  ],
  [
    {
      'exit': true,
      'regex': />/g,
      'style': 'sh_preproc'
    },
    {
      'next': 10,
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
      'next': 11,
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
      'next': 13,
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
      'regex': /\*\//g,
      'style': 'sh_comment'
    },
    {
      'next': 14,
      'regex': /\/\*/g,
      'style': 'sh_comment'
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
      'regex': /(?:TODO|FIXME)(?:[:]?)/g,
      'style': 'sh_todo'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 16,
      'regex': /</g,
      'style': 'sh_string'
    },
    {
      'next': 17,
      'regex': /"/g,
      'style': 'sh_string'
    },
    {
      'next': 18,
      'regex': /\/\/\//g,
      'style': 'sh_comment'
    },
    {
      'next': 24,
      'regex': /\/\//g,
      'style': 'sh_comment'
    },
    {
      'next': 25,
      'regex': /\/\*\*/g,
      'style': 'sh_comment'
    },
    {
      'next': 31,
      'regex': /\/\*/g,
      'style': 'sh_comment'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'exit': true,
      'regex': />/g,
      'style': 'sh_string'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
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
      'next': 19,
      'regex': /<!DOCTYPE/g,
      'state': 1,
      'style': 'sh_preproc'
    },
    {
      'next': 21,
      'regex': /<!--/g,
      'style': 'sh_comment'
    },
    {
      'regex': /<(?:\/)?[A-Za-z][A-Za-z0-9]*(?:\/)?>/g,
      'style': 'sh_keyword'
    },
    {
      'next': 22,
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
      'regex': /(?:TODO|FIXME)(?:[:]?)/g,
      'style': 'sh_todo'
    }
  ],
  [
    {
      'exit': true,
      'regex': />/g,
      'style': 'sh_preproc'
    },
    {
      'next': 20,
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
      'next': 21,
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
      'next': 23,
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
      'regex': /\*\//g,
      'style': 'sh_comment'
    },
    {
      'next': 25,
      'regex': /\/\*\*/g,
      'style': 'sh_comment'
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
      'next': 26,
      'regex': /<!DOCTYPE/g,
      'state': 1,
      'style': 'sh_preproc'
    },
    {
      'next': 28,
      'regex': /<!--/g,
      'style': 'sh_comment'
    },
    {
      'regex': /<(?:\/)?[A-Za-z][A-Za-z0-9]*(?:\/)?>/g,
      'style': 'sh_keyword'
    },
    {
      'next': 29,
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
      'regex': /(?:TODO|FIXME)(?:[:]?)/g,
      'style': 'sh_todo'
    }
  ],
  [
    {
      'exit': true,
      'regex': />/g,
      'style': 'sh_preproc'
    },
    {
      'next': 27,
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
      'next': 28,
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
      'next': 30,
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
      'regex': /\*\//g,
      'style': 'sh_comment'
    },
    {
      'next': 31,
      'regex': /\/\*/g,
      'style': 'sh_comment'
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
      'regex': /(?:TODO|FIXME)(?:[:]?)/g,
      'style': 'sh_todo'
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
  ]
];
