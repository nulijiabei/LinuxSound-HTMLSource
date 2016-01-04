if (! this.sh_languages) {
  this.sh_languages = {};
}
sh_languages['latex'] = [
  [
    {
      'next': 1,
      'regex': /%/g,
      'style': 'sh_comment'
    },
    {
      'regex': /&|~/g,
      'style': 'sh_symbol'
    },
    {
      'regex': /\\[$_&{}~^%#`'"|\s\\]/g,
      'style': 'sh_symbol'
    },
    {
      'next': 2,
      'regex': /"/g,
      'style': 'sh_string'
    },
    {
      'next': 3,
      'regex': /``/g,
      'style': 'sh_string'
    },
    {
      'next': 4,
      'regex': /`/g,
      'style': 'sh_string'
    },
    {
      'regex': /\$\$|\\\[|\\\]/g,
      'style': 'sh_math'
    },
    {
      'next': 5,
      'regex': /\$/g,
      'style': 'sh_math'
    },
    {
      'next': 6,
      'regex': /\\textit/g,
      'state': 1,
      'style': 'sh_keyword'
    },
    {
      'next': 9,
      'regex': /\\textbf/g,
      'state': 1,
      'style': 'sh_keyword'
    },
    {
      'next': 12,
      'regex': /\\texttt/g,
      'state': 1,
      'style': 'sh_keyword'
    },
    {
      'next': 15,
      'regex': /\\underline/g,
      'state': 1,
      'style': 'sh_keyword'
    },
    {
      'regex': /\\[A-Za-z]+/g,
      'style': 'sh_keyword'
    },
    {
      'regex': /\*/g,
      'style': 'sh_symbol'
    },
    {
      'regex': /\{[ \t]*$/g,
      'style': 'sh_normal'
    },
    {
      'next': 18,
      'regex': /\[/g,
      'style': 'sh_optionalargument'
    },
    {
      'next': 19,
      'regex': /\{/g,
      'style': 'sh_argument'
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
      'regex': /\\(?:\\|'')/g
    },
    {
      'exit': true,
      'regex': /''/g,
      'style': 'sh_string'
    }
  ],
  [
    {
      'regex': /\\(?:\\|')/g
    },
    {
      'exit': true,
      'regex': /'/g,
      'style': 'sh_string'
    }
  ],
  [
    {
      'regex': /\\(?:\\|\$)/g
    },
    {
      'exit': true,
      'regex': /\$/g,
      'style': 'sh_math'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 7,
      'regex': /\{/g,
      'style': 'sh_italics'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 8,
      'regex': /\{/g,
      'style': 'sh_italics'
    },
    {
      'exitall': true,
      'regex': /\}/g,
      'style': 'sh_italics'
    }
  ],
  [
    {
      'regex': /\\(?:\\|\})/g
    },
    {
      'exit': true,
      'regex': /\}/g,
      'style': 'sh_italics'
    },
    {
      'next': 8,
      'regex': /\{/g,
      'style': 'sh_italics'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 10,
      'regex': /\{/g,
      'style': 'sh_bold'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 11,
      'regex': /\{/g,
      'style': 'sh_bold'
    },
    {
      'exitall': true,
      'regex': /\}/g,
      'style': 'sh_bold'
    }
  ],
  [
    {
      'regex': /\\(?:\\|\})/g
    },
    {
      'exit': true,
      'regex': /\}/g,
      'style': 'sh_bold'
    },
    {
      'next': 11,
      'regex': /\{/g,
      'style': 'sh_bold'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 13,
      'regex': /\{/g,
      'style': 'sh_fixed'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 14,
      'regex': /\{/g,
      'style': 'sh_fixed'
    },
    {
      'exitall': true,
      'regex': /\}/g,
      'style': 'sh_fixed'
    }
  ],
  [
    {
      'regex': /\\(?:\\|\})/g
    },
    {
      'exit': true,
      'regex': /\}/g,
      'style': 'sh_fixed'
    },
    {
      'next': 14,
      'regex': /\{/g,
      'style': 'sh_fixed'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 16,
      'regex': /\{/g,
      'style': 'sh_underline'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 17,
      'regex': /\{/g,
      'style': 'sh_underline'
    },
    {
      'exitall': true,
      'regex': /\}/g,
      'style': 'sh_underline'
    }
  ],
  [
    {
      'regex': /\\(?:\\|\})/g
    },
    {
      'exit': true,
      'regex': /\}/g,
      'style': 'sh_underline'
    },
    {
      'next': 17,
      'regex': /\{/g,
      'style': 'sh_underline'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'regex': /\\(?:\\|\])/g
    },
    {
      'exit': true,
      'regex': /\]/g,
      'style': 'sh_optionalargument'
    }
  ],
  [
    {
      'regex': /\\(?:\\|\})/g
    },
    {
      'exit': true,
      'regex': /\}/g,
      'style': 'sh_argument'
    },
    {
      'next': 19,
      'regex': /\{/g,
      'style': 'sh_argument'
    }
  ]
];
