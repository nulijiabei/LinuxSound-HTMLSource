if (! this.sh_languages) {
  this.sh_languages = {};
}
sh_languages['changelog'] = [
  [
    {
      'next': 1,
      'regex': /[\d]{2,4}-?[\d]{2}-?[\d]{2}/g,
      'state': 1,
      'style': 'sh_date'
    },
    {
      'regex': /(^[ \t]+)(\*)([ \t]+)((?:[^:]+\:)?)/g,
      'style': ['sh_normal', 'sh_symbol', 'sh_normal', 'sh_file']
    },
    {
      'regex': /(^[ \t]+)((?:[^:]+\:)?)/g,
      'style': ['sh_normal', 'sh_file']
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
      'regex': /(?:[A-Za-z0-9_]|[`~!@#$%&*()_=+{}|;:",<.>\/?'\\[\]\^\-])+/g,
      'style': 'sh_name'
    }
  ]
];
