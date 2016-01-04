if (! this.sh_languages) {
  this.sh_languages = {};
}
sh_languages['log'] = [
  [
    {
      'next': 1,
      'regex': /^[A-Za-z]{3}[ \t]{1,2}[\d]{1,2}(?=[ \t][\d]{2}:[\d]{2}:[\d]{2})/g,
      'state': 1,
      'style': 'sh_date'
    },
    {
      'next': 6,
      'regex': /^[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}\b/g,
      'state': 1,
      'style': 'sh_ip'
    },
    {
      'next': 8,
      'regex': /^\[[A-Za-z]{3}[ \t][A-Za-z]{3}[ \t]{1,2}[\d]{1,2}[ \t](?=[\d]{2}:[\d]{2}:[\d]{2})/g,
      'state': 1,
      'style': 'sh_date'
    },
    {
      'regex': /[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}\b/g,
      'style': 'sh_ip'
    },
    {
      'regex': /\b(?:root|failure)\b/g,
      'style': 'sh_string'
    },
    {
      'regex': /((?:port|pid)[ \t])([\d]+)/g,
      'style': ['sh_normal', 'sh_port']
    },
    {
      'next': 9,
      'regex': /[ \t](?=(?:IN|OUT)=)/g,
      'state': 1,
      'style': 'sh_normal'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 2,
      'regex': /\b[\d]{2}:[\d]{2}:[\d]{2}\b/g,
      'state': 1,
      'style': 'sh_time'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 3,
      'regex': /[^ \t]+/g,
      'state': 1,
      'style': 'sh_symbol'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'exitall': true,
      'regex': /:/g,
      'style': 'sh_normal'
    },
    {
      'regex': /[^:\(\[]+/g,
      'style': 'sh_function'
    },
    {
      'next': 4,
      'regex': /\[/g,
      'style': 'sh_number'
    },
    {
      'next': 5,
      'regex': /\(/g,
      'style': 'sh_number'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'exit': true,
      'regex': /\]/g,
      'style': 'sh_number'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'exit': true,
      'regex': /\)/g,
      'style': 'sh_number'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'regex': /[A-Za-z0-9]+(?=[ \t]\[[\d]{2}\/[A-Za-z]{3}\/[\d]{4})/g,
      'style': 'sh_string'
    },
    {
      'regex': /[\d]{2}\/[A-Za-z]{3}\/[\d]{4}(?=:[\d]{2}:[\d]{2}:[\d]{2})/g,
      'style': 'sh_date'
    },
    {
      'regex': /[\d]{2}:[\d]{2}:[\d]{2}[ \t][+-][\d]{4}/g,
      'style': 'sh_time'
    },
    {
      'regex': /[1-5][\d]{2}[ \t][-0-9]+/g,
      'style': 'sh_twonumbers'
    },
    {
      'next': 7,
      'regex': /\b(?:OPTIONS|GET|HEAD|POST|PUT|DELETE|TRACE|CONNECT|PROPFIND|MKCOL|COPY|MOVE|LOCK|UNLOCK)\b/g,
      'state': 1,
      'style': 'sh_webmethod'
    }
  ],
  [
    {
      'exit': true,
      'regex': /[^ \t]+/g,
      'style': 'sh_string'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'regex': /\b[\d]{2}:[\d]{2}:[\d]{2}\b/g,
      'style': 'sh_time'
    },
    {
      'regex': /[\d]{4}\]/g,
      'style': 'sh_date'
    },
    {
      'regex': /\[[A-Za-z]{3}[ \t][A-Za-z]{3}[ \t]{1,2}[\d]{1,2}[ \t](?=[\d]{2}:[\d]{2}:[\d]{2})/g,
      'style': 'sh_date'
    },
    {
      'regex': /\[error\]/g,
      'style': 'sh_string'
    },
    {
      'regex': /\[notice\]/g,
      'style': 'sh_comment'
    },
    {
      'regex': /[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}\b/g,
      'style': 'sh_ip'
    }
  ],
  [
    {
      'exit': true,
      'regex': /$/g
    },
    {
      'next': 10,
      'regex': /(?:IN|OUT|PROTO)=(?=[^ \t]+)/g,
      'state': 1,
      'style': 'sh_normal'
    },
    {
      'next': 11,
      'regex': /(?:SPT|DPT|TYPE|SEQ)=(?=[^ \t]+)/g,
      'state': 1,
      'style': 'sh_normal'
    },
    {
      'regex': /\b(?:CWR|ECE|URG|ACK|PSH|RST|SYN|FIN)\b/g,
      'style': 'sh_number'
    },
    {
      'regex': /[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}\b/g,
      'style': 'sh_ip'
    }
  ],
  [
    {
      'exit': true,
      'regex': /[^ \t]+/g,
      'style': 'sh_string'
    }
  ],
  [
    {
      'exit': true,
      'regex': /[^ \t]+/g,
      'style': 'sh_cbracket'
    }
  ]
];
